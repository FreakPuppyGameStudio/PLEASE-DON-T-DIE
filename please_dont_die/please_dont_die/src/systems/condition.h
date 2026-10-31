#pragma once
// Tiny condition language used by data files, e.g.
//   "minutes>=720 and flag:evolved"   "!flag:mem_mom"   "neural_interface>=1"
// Names: vitality oxygenation immunity neural organ biomass energy integrity
//        adaptation minutes day memories nutrients regen camo neural_interface
// An empty expression is always true.

#include <string>

#include "simulation/simulation.h"

bool evalCondition(const std::string& expr, const GameState& s);
