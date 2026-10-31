#include "terminal/commands.h"

#include "save/save_system.h"
#include "systems/evolution_system.h"
#include "systems/time_system.h"
#include "terminal/actions.h"
#include "terminal/command_util.h"

namespace {

void cmdStatus(const GameState& s, CommandResult& r) {
    say(r, LineKind::System, clockText(s));
    say(r, LineKind::System, remainingText(s));
    say(r, LineKind::Narrative, "HOST: ANASTASIA VOLKOVA");
    say(r, LineKind::Info, "  Vitality: " + numText(s.vitality));
    say(r, LineKind::Info, "  Oxygenation: " + numText(s.oxygenation));
    say(r, LineKind::Info, "  Immunity: " + numText(s.immunity));
    say(r, LineKind::Info, "  Neural Stability: " + numText(s.neural));
    say(r, LineKind::Info, "  Organ Integrity: " + numText(s.organ));
    say(r, LineKind::Narrative, "PARASITE: S.O.R.A.");
    say(r, LineKind::Info, "  Biomass: " + numText(s.biomass));
    say(r, LineKind::Info, "  Energy: " + numText(s.energy));
    say(r, LineKind::Info, "  Integrity: " + numText(s.integrity));
    say(r, LineKind::Info, "  Adaptation: " + numText(s.adaptation));
}

void cmdInventory(const GameState& s, const Content& c, CommandResult& r) {
    say(r, LineKind::System, "INVENTORY");
    say(r, LineKind::Info, "  Biomass: " + numText(s.biomass));
    say(r, LineKind::Info, "  Energy: " + numText(s.energy));
    say(r, LineKind::Info, "  Adaptation: " + numText(s.adaptation));
    say(r, LineKind::Info, "  Nutrient bags (IV line): " + std::to_string(s.nutrientBags));
    say(r, LineKind::Info, "  Memory fragments recovered: " + std::to_string(s.memoriesFound) + "/" +
                               std::to_string(c.memories.size()));
    for (const auto& b : c.branches) {
        say(r, LineKind::Info, "  " + b.name + " level: " + std::to_string(evolutionLevel(s, b.stat)) + "/" +
                                   std::to_string(b.levels.size()));
    }
}

void cmdHelp(CommandResult& r) {
    say(r, LineKind::System, "AVAILABLE COMMANDS");
    say(r, LineKind::Info, "  STATUS                          Host and S.O.R.A. readings. (0 min)");
    say(r, LineKind::Info, "  INVENTORY                       Resources and evolution levels. (0 min)");
    say(r, LineKind::Info, "  DIAGNOSE [HEART|BRAIN|LUNGS|IMMUNE|ALL]     (30 min)");
    say(r, LineKind::Info, "  HEAL ORGAN|NEURAL|OXYGEN                    (60 min)");
    say(r, LineKind::Info, "  ABSORB NUTRIENTS|TISSUE                     (45 min)");
    say(r, LineKind::Info, "      TISSUE gives more biomass but harms Anastasia.");
    say(r, LineKind::Info, "  STABILIZE IMMUNE|NEURAL|ORGAN               (45 min)");
    say(r, LineKind::Info, "  EVOLVE REGENERATION|CAMOUFLAGE|NEURAL INTERFACE   (120 min)");
    say(r, LineKind::Info, "  COMMUNICATE                     Search for memory fragments. (30 min)");
    say(r, LineKind::Info, "  WAIT [minutes]                  Let time pass. Default 60, range 10-1440.");
    say(r, LineKind::Info, "  DECIDE PROTECT|CLAIM|RELEASE    Final decision, at hour 66 only.");
    say(r, LineKind::Info, "  SAVE / LOAD                     Write or restore the session.");
    say(r, LineKind::Info, "  CLEAR / EXIT                    Clear the screen or close the session.");
    say(r, LineKind::Info, "  F2 toggles the music.");
}

} // namespace

std::vector<OutLine> introLines(const GameState& s) {
    return {
        {LineKind::System, "PLEASE, DON'T DIE"},
        {LineKind::System, "S.O.R.A. BIOLOGICAL INTERFACE v0.1"},
        {LineKind::Info, ""},
        {LineKind::Info, "HOST: ANASTASIA VOLKOVA"},
        {LineKind::Info, "LOCATION: INTENSIVE CARE UNIT"},
        {LineKind::Info, ""},
        {LineKind::System, clockText(s) + "  /  72 HOURS"},
        {LineKind::System, "SYSTEM: Consciousness initialized."},
        {LineKind::System, "SYSTEM: Host condition unstable."},
        {LineKind::System, "SYSTEM: Symbiotic integration incomplete."},
        {LineKind::Info, ""},
        {LineKind::Narrative, "S.O.R.A.: Where... am I?"},
        {LineKind::System, "SYSTEM: You are inside your assigned host."},
        {LineKind::Narrative, "S.O.R.A.: Assigned... host?"},
        {LineKind::Info, ""},
        {LineKind::Info, "Type HELP to list the available commands."},
    };
}

CommandResult executeCommand(GameState& s, const Content& c, const std::string& raw) {
    CommandResult r;
    const std::string line = upperCase(trimSpaces(raw));
    if (line.empty()) return r;

    const size_t sp = line.find(' ');
    const std::string cmd = sp == std::string::npos ? line : line.substr(0, sp);
    const std::string arg = sp == std::string::npos ? "" : trimSpaces(line.substr(sp + 1));

    // Commands that work even after the session has ended.
    if (cmd == "CLEAR") { r.clearScreen = true; return r; }
    if (cmd == "EXIT") {
        say(r, LineKind::Info, "Session closed.");
        r.exitRequested = true;
        return r;
    }
    if (cmd == "HELP") { cmdHelp(r); return r; }
    if (cmd == "STATUS") { cmdStatus(s, r); return r; }
    if (cmd == "INVENTORY") { cmdInventory(s, c, r); return r; }
    if (cmd == "LOAD") {
        std::string err;
        if (loadGame(s, SAVE_PATH, err)) {
            say(r, LineKind::System, "Game loaded.");
            say(r, LineKind::System, clockText(s));
        } else {
            say(r, LineKind::Error, "LOAD failed: " + err);
        }
        return r;
    }
    if (cmd == "SAVE") {
        if (s.over) {
            say(r, LineKind::Error, "Cannot save a finished session.");
            return r;
        }
        std::string err;
        if (saveGame(s, SAVE_PATH, err)) say(r, LineKind::System, "Game saved to " + SAVE_PATH + ".");
        else say(r, LineKind::Error, "SAVE failed: " + err);
        return r;
    }

    if (s.over) {
        say(r, LineKind::Error, "SESSION TERMINATED. Restart the application or LOAD a save.");
        return r;
    }

    if (cmd == "DECIDE") {
        runDecision(s, c, arg, r);
        return r;
    }
    if (!runTimedAction(s, c, cmd, arg, r)) {
        say(r, LineKind::Error, "Unknown command: " + cmd + ". Type HELP for the list of commands.");
    }
    return r;
}
