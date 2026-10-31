#pragma once
// Core state of a session. Pure data and biology: no Raylib, no file access.

#include <set>
#include <string>
#include <vector>

enum class LineKind { System, Narrative, Warning, Error, Player, Info };

struct OutLine {
    LineKind kind;
    std::string text;
};

constexpr int TOTAL_MINUTES = 72 * 60;     // 72 hours of in-game time
constexpr int DECISION_MINUTE = 66 * 60;   // the clock stops here for the final decision
constexpr int MAX_EVOLUTION = 2;

struct GameState {
    // Anastasia Volkova (host)
    double vitality = 78;
    double oxygenation = 70;
    double immunity = 25;
    double neural = 45;
    double organ = 66;

    // S.O.R.A. (parasite)
    double biomass = 40;
    double energy = 60;
    double integrity = 85;
    double adaptation = 10;

    // Progress
    int nutrientBags = 4;
    int minutes = 0;
    int evoRegen = 0;
    int evoCamo = 0;
    int evoNeural = 0;
    int memoriesFound = 0;
    int tissueAbsorbed = 0;
    int immuneStabilized = 0;

    bool over = false;
    std::string ending;
    std::set<std::string> flags;   // story flags: mem_<id>, evolved, decision_open, choice_*...
    std::set<std::string> fired;   // events that already happened
};

void clampState(GameState& s);

// One simulated hour is advanced by `hours`. Called in 10-minute steps by the time system.
void tickBiology(GameState& s, double hours);
