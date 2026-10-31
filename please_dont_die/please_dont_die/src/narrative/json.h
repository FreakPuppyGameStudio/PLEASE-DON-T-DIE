#pragma once
// Minimal JSON reader used for the data/ files. Supports objects, arrays,
// strings (with \uXXXX escapes), numbers, booleans and null.

#include <string>
#include <utility>
#include <vector>

namespace mini_json {

struct Value {
    enum class Type { Null, Bool, Number, String, Array, Object };

    Type type = Type::Null;
    bool boolean = false;
    double number = 0.0;
    std::string str;
    std::vector<Value> items;                            // Array
    std::vector<std::pair<std::string, Value>> members;  // Object, in file order

    const Value* find(const std::string& key) const;
    std::string stringOr(const std::string& key, const std::string& fallback) const;
    double numberOr(const std::string& key, double fallback) const;
};

bool parse(const std::string& text, Value& out, std::string& err);

} // namespace mini_json
