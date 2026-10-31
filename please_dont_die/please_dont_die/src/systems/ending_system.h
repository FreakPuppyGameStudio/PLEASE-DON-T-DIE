#pragma once
// Decides the ending once the host or S.O.R.A. dies, or the 72 hours run out.

#include <vector>

#include "narrative/content.h"
#include "simulation/simulation.h"

// Returns true and writes the ending text if the game is over.
bool resolveEnding(GameState& s, const Content& c, std::vector<OutLine>& out);
