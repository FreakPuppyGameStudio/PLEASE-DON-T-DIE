#include "save/save_system.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <map>

namespace fs = std::filesystem;

bool saveGame(const GameState& s, const std::string& path, std::string& err) {
    try {
        const fs::path p(path);
        if (p.has_parent_path()) fs::create_directories(p.parent_path());
    } catch (const std::exception& e) {
        err = e.what();
        return false;
    }

    std::ofstream f(path, std::ios::trunc);
    if (!f) {
        err = "could not write " + path;
        return false;
    }

    f.precision(12);
    f << "version 1\n"
      << "vitality " << s.vitality << "\n"
      << "oxygenation " << s.oxygenation << "\n"
      << "immunity " << s.immunity << "\n"
      << "neural " << s.neural << "\n"
      << "organ " << s.organ << "\n"
      << "biomass " << s.biomass << "\n"
      << "energy " << s.energy << "\n"
      << "integrity " << s.integrity << "\n"
      << "adaptation " << s.adaptation << "\n"
      << "nutrientBags " << s.nutrientBags << "\n"
      << "minutes " << s.minutes << "\n"
      << "evoRegen " << s.evoRegen << "\n"
      << "evoCamo " << s.evoCamo << "\n"
      << "evoNeural " << s.evoNeural << "\n"
      << "memoriesFound " << s.memoriesFound << "\n"
      << "tissueAbsorbed " << s.tissueAbsorbed << "\n"
      << "immuneStabilized " << s.immuneStabilized << "\n";

    for (const auto& flag : s.flags) f << "flag " << flag << "\n";
    for (const auto& ev : s.fired) f << "fired " << ev << "\n";

    if (!f) {
        err = "write failed for " + path;
        return false;
    }
    return true;
}

bool loadGame(GameState& s, const std::string& path, std::string& err) {
    std::ifstream f(path);
    if (!f) {
        err = "no save file found at " + path;
        return false;
    }

    GameState n;
    std::map<std::string, double*> reals = {
        {"vitality", &n.vitality},     {"oxygenation", &n.oxygenation}, {"immunity", &n.immunity},
        {"neural", &n.neural},         {"organ", &n.organ},             {"biomass", &n.biomass},
        {"energy", &n.energy},         {"integrity", &n.integrity},     {"adaptation", &n.adaptation},
    };
    std::map<std::string, int*> ints = {
        {"nutrientBags", &n.nutrientBags},     {"minutes", &n.minutes},
        {"evoRegen", &n.evoRegen},             {"evoCamo", &n.evoCamo},
        {"evoNeural", &n.evoNeural},           {"memoriesFound", &n.memoriesFound},
        {"tissueAbsorbed", &n.tissueAbsorbed}, {"immuneStabilized", &n.immuneStabilized},
    };

    std::string key;
    while (f >> key) {
        if (key == "flag" || key == "fired") {
            std::string value;
            if (!(f >> value)) break;
            if (key == "flag") n.flags.insert(value);
            else n.fired.insert(value);
        } else if (auto r = reals.find(key); r != reals.end()) {
            if (!(f >> *r->second)) {
                err = "corrupt value for " + key;
                return false;
            }
        } else if (auto i = ints.find(key); i != ints.end()) {
            if (!(f >> *i->second)) {
                err = "corrupt value for " + key;
                return false;
            }
        } else {
            std::string ignored;
            std::getline(f, ignored);  // unknown key or the version line
        }
    }

    clampState(n);
    n.over = false;
    n.ending.clear();
    s = n;
    return true;
}
