#pragma once
// Turn-based clock. Time only moves when an action or WAIT spends it.

#include <string>
#include <vector>

#include "narrative/content.h"
#include "simulation/simulation.h"

std::string clockText(const GameState& s);      // "DAY 1 - 00:30"
std::string remainingText(const GameState& s);  // "TIME REMAINING: 71 HOURS 30 MINUTES."

// True while the clock is held at DECISION_MINUTE waiting for DECIDE.
bool decisionPending(const GameState& s);

// Advances the clock in 10-minute steps, applying biology, events and endings.
void advanceTime(GameState& s, int minutes, const Content& c, std::vector<OutLine>& out);
