#pragma once
// Time-consuming player actions and the final decision.

#include <string>

#include "narrative/content.h"
#include "simulation/simulation.h"
#include "terminal/commands.h"

// Runs DIAGNOSE, HEAL, ABSORB, STABILIZE, EVOLVE, COMMUNICATE or WAIT.
// Returns false if cmd is not one of these.
bool runTimedAction(GameState& s, const Content& c, const std::string& cmd, const std::string& arg,
                    CommandResult& r);

// Runs DECIDE PROTECT | CLAIM | RELEASE.
void runDecision(GameState& s, const Content& c, const std::string& arg, CommandResult& r);
