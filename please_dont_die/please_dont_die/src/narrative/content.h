#pragma once
// Game content loaded from data/*.json. Rules stay in code; words, numbers and
// event triggers live in data so they can be tuned without recompiling.

#include <map>
#include <string>
#include <vector>

#include "simulation/simulation.h"

struct EventDef {
    std::string id;
    std::string when;  // condition expression, see systems/condition.h
    LineKind kind = LineKind::Narrative;
    std::string text;
};

struct MemoryDef {
    std::string id;
    std::string ru;
    std::string en;
    int confidence = 50;
    std::string requires_;  // condition expression, empty means always available
};

struct BranchLevel {
    double biomass = 0;
    double energy = 0;
    double adaptation = 0;
    int minutes = 120;
    std::string text;
};

struct BranchDef {
    std::string id;
    std::string name;
    std::string stat;  // "regen", "camo" or "neural_interface"
    std::vector<BranchLevel> levels;
};

struct Content {
    std::vector<EventDef> events;
    std::vector<MemoryDef> memories;
    std::vector<BranchDef> branches;
    std::map<std::string, std::vector<std::string>> dialogue;
};

// Loads events.json, memories.json, upgrades.json and dialogue.json from dataDir.
bool loadContent(const std::string& dataDir, Content& out, std::string& err);

// Returns the dialogue lines stored under key, or an empty list.
std::vector<std::string> dialogueLines(const Content& c, const std::string& key);
