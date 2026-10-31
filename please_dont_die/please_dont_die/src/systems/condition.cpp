#include "systems/condition.h"

#include <cstdlib>

namespace {

std::string trim(const std::string& s) {
    const size_t b = s.find_first_not_of(" \t");
    if (b == std::string::npos) return "";
    const size_t e = s.find_last_not_of(" \t");
    return s.substr(b, e - b + 1);
}

bool statValue(const std::string& name, const GameState& s, double& out) {
    if (name == "vitality") out = s.vitality;
    else if (name == "oxygenation") out = s.oxygenation;
    else if (name == "immunity") out = s.immunity;
    else if (name == "neural") out = s.neural;
    else if (name == "organ") out = s.organ;
    else if (name == "biomass") out = s.biomass;
    else if (name == "energy") out = s.energy;
    else if (name == "integrity") out = s.integrity;
    else if (name == "adaptation") out = s.adaptation;
    else if (name == "minutes") out = s.minutes;
    else if (name == "day") out = s.minutes / (24 * 60) + 1;
    else if (name == "memories") out = s.memoriesFound;
    else if (name == "nutrients") out = s.nutrientBags;
    else if (name == "regen") out = s.evoRegen;
    else if (name == "camo") out = s.evoCamo;
    else if (name == "neural_interface") out = s.evoNeural;
    else return false;
    return true;
}

bool evalTerm(const std::string& raw, const GameState& s) {
    const std::string term = trim(raw);
    if (term.empty()) return true;
    if (term[0] == '!') return !evalTerm(term.substr(1), s);
    if (term.rfind("flag:", 0) == 0) return s.flags.count(term.substr(5)) > 0;

    static const char* const ops[] = {">=", "<=", ">", "<", "="};
    for (const char* op : ops) {
        const size_t pos = term.find(op);
        if (pos == std::string::npos) continue;

        const std::string name = trim(term.substr(0, pos));
        const std::string rhs = trim(term.substr(pos + std::char_traits<char>::length(op)));
        char* end = nullptr;
        const double want = std::strtod(rhs.c_str(), &end);
        if (end == rhs.c_str() || *end != '\0') return false;

        double have = 0;
        if (!statValue(name, s, have)) return false;

        const std::string o = op;
        if (o == ">=") return have >= want;
        if (o == "<=") return have <= want;
        if (o == ">") return have > want;
        if (o == "<") return have < want;
        return have == want;
    }
    return false;
}

} // namespace

bool evalCondition(const std::string& expr, const GameState& s) {
    if (trim(expr).empty()) return true;
    size_t start = 0;
    while (true) {
        const size_t and_pos = expr.find(" and ", start);
        const std::string part = expr.substr(start, and_pos == std::string::npos ? std::string::npos : and_pos - start);
        if (!evalTerm(part, s)) return false;
        if (and_pos == std::string::npos) return true;
        start = and_pos + 5;
    }
}
