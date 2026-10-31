#pragma once
// Evolution branches. Names, costs and text come from data/upgrades.json; the
// numeric effects of each branch live here.

#include <string>

#include "narrative/content.h"
#include "simulation/simulation.h"

int evolutionLevel(const GameState& s, const std::string& stat);
void raiseEvolution(GameState& s, const std::string& stat);

// Matches a full branch name ("CAMOUFLAGE") or a prefix of at least four letters.
const BranchDef* findBranch(const Content& c, const std::string& arg);

double repairMultiplier(const GameState& s);    // REGENERATION
double immuneGrowthFactor(const GameState& s);  // CAMOUFLAGE
int camouflageBonus(const GameState& s);        // extra immune suppression from CAMOUFLAGE
double neuralGainPerHour(const GameState& s);   // NEURAL INTERFACE
