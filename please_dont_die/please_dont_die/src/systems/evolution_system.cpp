#include "systems/evolution_system.h"

int evolutionLevel(const GameState& s, const std::string& stat) {
    if (stat == "regen") return s.evoRegen;
    if (stat == "camo") return s.evoCamo;
    if (stat == "neural_interface") return s.evoNeural;
    return 0;
}

void raiseEvolution(GameState& s, const std::string& stat) {
    if (stat == "regen") s.evoRegen++;
    else if (stat == "camo") s.evoCamo++;
    else if (stat == "neural_interface") s.evoNeural++;
}

const BranchDef* findBranch(const Content& c, const std::string& arg) {
    for (const auto& b : c.branches) {
        if (arg == b.name) return &b;
        if (arg.size() >= 4 && b.name.rfind(arg, 0) == 0) return &b;
    }
    return nullptr;
}

double repairMultiplier(const GameState& s) { return 1.0 + 0.25 * s.evoRegen; }
double immuneGrowthFactor(const GameState& s) { return 1.0 - 0.25 * s.evoCamo; }
int camouflageBonus(const GameState& s) { return 5 * s.evoCamo; }
double neuralGainPerHour(const GameState& s) { return 0.3 * s.evoNeural; }
