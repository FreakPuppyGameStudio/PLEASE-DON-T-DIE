#pragma once
// Fires narrative events from data/events.json. Each event fires at most once.

#include <vector>

#include "narrative/content.h"
#include "simulation/simulation.h"

void checkEvents(GameState& s, const Content& c, std::vector<OutLine>& out);
