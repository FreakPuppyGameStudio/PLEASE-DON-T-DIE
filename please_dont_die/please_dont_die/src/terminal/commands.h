#pragma once
// Public interface of the command layer: what the terminal calls.

#include <string>
#include <vector>

#include "narrative/content.h"
#include "simulation/simulation.h"

struct CommandResult {
    std::vector<OutLine> lines;
    bool clearScreen = false;
    bool exitRequested = false;
};

// Parses and runs one player command (case-insensitive).
CommandResult executeCommand(GameState& s, const Content& c, const std::string& raw);

// Opening screen of a new session.
std::vector<OutLine> introLines(const GameState& s);
