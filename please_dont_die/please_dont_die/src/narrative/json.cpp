#include "narrative/json.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

namespace mini_json {

const Value* Value::find(const std::string& key) const {
    for (const auto& m : members) {
        if (m.first == key) return &m.second;
    }
    return nullptr;
}

std::string Value::stringOr(const std::string& key, const std::string& fallback) const {
    const Value* v = find(key);
    return (v && v->type == Type::String) ? v->str : fallback;
}

double Value::numberOr(const std::string& key, double fallback) const {
    const Value* v = find(key);
    return (v && v->type == Type::Number) ? v->number : fallback;
}

namespace {

void appendUtf8(std::string& out, unsigned cp) {
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

class Parser {
public:
    explicit Parser(const std::string& text) : t_(text) {}

    bool document(Value& out) {
        if (!value(out)) return false;
        skipWs();
        if (p_ != t_.size()) return fail("trailing characters");
        return true;
    }

    const std::string& error() const { return err_; }

private:
    const std::string& t_;
    size_t p_ = 0;
    std::string err_;

    bool fail(const std::string& msg) {
        if (err_.empty()) err_ = msg + " at offset " + std::to_string(p_);
        return false;
    }

    void skipWs() {
        while (p_ < t_.size() && std::isspace(static_cast<unsigned char>(t_[p_]))) ++p_;
    }

    bool literal(const char* word) {
        const size_t n = std::strlen(word);
        if (t_.compare(p_, n, word) == 0) {
            p_ += n;
            return true;
        }
        return false;
    }

    bool value(Value& v) {
        skipWs();
        if (p_ >= t_.size()) return fail("unexpected end of input");
        const char c = t_[p_];
        if (c == '{') return object(v);
        if (c == '[') return array(v);
        if (c == '"') {
            v.type = Value::Type::String;
            return parseString(v.str);
        }
        if (literal("true")) { v.type = Value::Type::Bool; v.boolean = true; return true; }
        if (literal("false")) { v.type = Value::Type::Bool; v.boolean = false; return true; }
        if (literal("null")) { v.type = Value::Type::Null; return true; }
        return number(v);
    }

    bool parseString(std::string& out) {
        ++p_;  // opening quote
        while (p_ < t_.size()) {
            const char c = t_[p_++];
            if (c == '"') return true;
            if (c != '\\') {
                out += c;
                continue;
            }
            if (p_ >= t_.size()) break;
            const char e = t_[p_++];
            switch (e) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'u': {
                    if (p_ + 4 > t_.size()) return fail("bad unicode escape");
                    char* end = nullptr;
                    const std::string hex = t_.substr(p_, 4);
                    const unsigned long cp = std::strtoul(hex.c_str(), &end, 16);
                    if (*end != '\0') return fail("bad unicode escape");
                    p_ += 4;
                    appendUtf8(out, static_cast<unsigned>(cp));
                    break;
                }
                default:
                    return fail("bad escape sequence");
            }
        }
        return fail("unterminated string");
    }

    bool number(Value& v) {
        const size_t start = p_;
        while (p_ < t_.size()) {
            const char c = t_[p_];
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '+' ||
                c == '.' || c == 'e' || c == 'E') {
                ++p_;
            } else {
                break;
            }
        }
        if (p_ == start) return fail("unexpected character");
        const std::string tok = t_.substr(start, p_ - start);
        char* end = nullptr;
        v.number = std::strtod(tok.c_str(), &end);
        if (end == tok.c_str() || *end != '\0') return fail("bad number");
        v.type = Value::Type::Number;
        return true;
    }

    bool object(Value& v) {
        v.type = Value::Type::Object;
        ++p_;
        skipWs();
        if (p_ < t_.size() && t_[p_] == '}') {
            ++p_;
            return true;
        }
        while (true) {
            skipWs();
            if (p_ >= t_.size() || t_[p_] != '"') return fail("expected object key");
            std::string key;
            if (!parseString(key)) return false;
            skipWs();
            if (p_ >= t_.size() || t_[p_] != ':') return fail("expected ':'");
            ++p_;
            Value child;
            if (!value(child)) return false;
            v.members.emplace_back(std::move(key), std::move(child));
            skipWs();
            if (p_ >= t_.size()) return fail("unterminated object");
            if (t_[p_] == ',') { ++p_; continue; }
            if (t_[p_] == '}') { ++p_; return true; }
            return fail("expected ',' or '}'");
        }
    }

    bool array(Value& v) {
        v.type = Value::Type::Array;
        ++p_;
        skipWs();
        if (p_ < t_.size() && t_[p_] == ']') {
            ++p_;
            return true;
        }
        while (true) {
            Value child;
            if (!value(child)) return false;
            v.items.push_back(std::move(child));
            skipWs();
            if (p_ >= t_.size()) return fail("unterminated array");
            if (t_[p_] == ',') { ++p_; continue; }
            if (t_[p_] == ']') { ++p_; return true; }
            return fail("expected ',' or ']'");
        }
    }
};

} // namespace

bool parse(const std::string& text, Value& out, std::string& err) {
    Parser p(text);
    if (!p.document(out)) {
        err = p.error();
        return false;
    }
    return true;
}

} // namespace mini_json
