#pragma once
// Small helpers shared by the command handlers.

#include <cctype>
#include <cmath>
#include <string>
#include <vector>

#include "systems/time_system.h"
#include "terminal/commands.h"

inline std::string upperCase(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

inline std::string trimSpaces(const std::string& s) {
    const size_t b = s.find_first_not_of(" \t");
    if (b == std::string::npos) return "";
    const size_t e = s.find_last_not_of(" \t");
    return s.substr(b, e - b + 1);
}

inline std::string numText(double v) {
    return std::to_string(static_cast<int>(std::lround(v)));
}

inline void say(CommandResult& r, LineKind kind, const std::string& text) {
    r.lines.push_back({kind, text});
}

inline void sayAll(CommandResult& r, LineKind kind, const std::vector<std::string>& texts) {
    for (const auto& t : texts) say(r, kind, t);
}

// Every time-consuming action ends here: report the cost, then run the clock.
inline void spend(GameState& s, const Content& c, CommandResult& r, int minutes) {
    clampState(s);
    say(r, LineKind::Info, "Time consumed: " + std::to_string(minutes) + " minutes.");
    advanceTime(s, minutes, c, r.lines);
}

inline bool canAfford(const GameState& s, CommandResult& r, double biomass, double energy) {
    if (s.biomass < biomass) {
        say(r, LineKind::Error, "Insufficient biomass: need " + numText(biomass) + ", have " + numText(s.biomass) + ".");
        return false;
    }
    if (s.energy < energy) {
        say(r, LineKind::Error, "Insufficient energy: need " + numText(energy) + ", have " + numText(s.energy) + ".");
        return false;
    }
    return true;
}
