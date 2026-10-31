#pragma once
// SAVE and LOAD: a plain-text "key value" file, one entry per line.

#include <string>

#include "simulation/simulation.h"

const std::string SAVE_PATH = "saves/savegame.txt";

bool saveGame(const GameState& s, const std::string& path, std::string& err);
bool loadGame(GameState& s, const std::string& path, std::string& err);
