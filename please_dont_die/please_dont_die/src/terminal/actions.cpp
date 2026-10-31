#include "terminal/actions.h"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "systems/condition.h"
#include "systems/evolution_system.h"
#include "systems/time_system.h"
#include "terminal/command_util.h"

namespace {

void diagnose(GameState& s, const Content& c, const std::string& arg, CommandResult& r) {
    const std::string target = arg.empty() ? "ALL" : arg;
    const bool all = target == "ALL";
    if (!all && target != "HEART" && target != "BRAIN" && target != "LUNGS" && target != "IMMUNE") {
        say(r, LineKind::Error, "Unknown diagnostic target: " + arg + ". Options: HEART, BRAIN, LUNGS, IMMUNE, ALL.");
        return;
    }

    say(r, LineKind::System, "SCANNING " + target + " TISSUE...");
    say(r, LineKind::Info, "RESULT:");
    s.flags.insert("diagnosed");

    if (all || target == "HEART") {
        say(r, LineKind::Info, "  Organ Integrity: " + numText(s.organ));
        if (s.organ < 50.0) say(r, LineKind::Warning, "  Cardiac degradation detected.");
    }
    if (all || target == "LUNGS") {
        say(r, LineKind::Info, "  Oxygenation: " + numText(s.oxygenation));
        if (s.oxygenation < 50.0) say(r, LineKind::Warning, "  Hypoxic tissue detected.");
    }
    if (all || target == "BRAIN") {
        say(r, LineKind::Info, "  Neural Stability: " + numText(s.neural));
        if (s.neural < 40.0) say(r, LineKind::Warning, "  Neural instability detected.");
    }
    if (all || target == "IMMUNE") {
        say(r, LineKind::Info, "  Immunity: " + numText(s.immunity));
        if (s.immunity > 50.0) say(r, LineKind::Warning, "  Immune rejection elevated.");
    }
    if (all) say(r, LineKind::Info, "  S.O.R.A. Integrity: " + numText(s.integrity));

    spend(s, c, r, 30);
}

void heal(GameState& s, const Content& c, const std::string& arg, CommandResult& r) {
    double GameState::*field = nullptr;
    double gain = 0.0, biomass = 0.0, energy = 0.0;
    std::string key;
    if (arg == "ORGAN") {
        field = &GameState::organ; gain = 20; biomass = 15; energy = 10; key = "heal.organ";
    } else if (arg == "NEURAL") {
        field = &GameState::neural; gain = 15; biomass = 12; energy = 12; key = "heal.neural";
    } else if (arg == "OXYGEN") {
        field = &GameState::oxygenation; gain = 25; biomass = 10; energy = 8; key = "heal.oxygen";
    } else {
        say(r, LineKind::Error, "HEAL needs a target: ORGAN, NEURAL or OXYGEN.");
        return;
    }
    if (!canAfford(s, r, biomass, energy)) return;

    // Efficiency depends on S.O.R.A.'s integrity and on the REGENERATION level.
    const double eff = (0.5 + s.integrity / 200.0) * repairMultiplier(s);
    s.biomass -= biomass;
    s.energy -= energy;
    s.*field += gain * eff;

    sayAll(r, LineKind::System, dialogueLines(c, key));
    say(r, LineKind::Info, "Repair efficiency: " + numText(eff * 100) + "%");
    spend(s, c, r, 60);
}

void absorb(GameState& s, const Content& c, const std::string& arg, CommandResult& r) {
    if (arg == "NUTRIENTS") {
        if (s.nutrientBags <= 0) {
            say(r, LineKind::Error, "No nutrient bags remain on the IV line.");
            return;
        }
        s.nutrientBags--;
        s.biomass += 10;
        s.energy += 8;
        sayAll(r, LineKind::System, dialogueLines(c, "absorb.nutrients"));
        spend(s, c, r, 45);
    } else if (arg == "TISSUE") {
        s.biomass += 25;
        s.energy += 4;
        s.organ -= 5;
        s.vitality -= 4;
        s.adaptation += 4;
        s.tissueAbsorbed++;
        s.flags.insert("absorbed_tissue");
        sayAll(r, LineKind::Warning, dialogueLines(c, "absorb.tissue"));
        spend(s, c, r, 45);
    } else {
        say(r, LineKind::Error, "ABSORB needs a source: NUTRIENTS or TISSUE.");
    }
}

void stabilize(GameState& s, const Content& c, const std::string& arg, CommandResult& r) {
    if (arg == "IMMUNE") {
        if (!canAfford(s, r, 0, 15)) return;
        s.energy -= 15;
        s.immunity -= 15 + camouflageBonus(s);
        s.immuneStabilized++;
        sayAll(r, LineKind::System, dialogueLines(c, "stabilize.immune"));
        spend(s, c, r, 45);
    } else if (arg == "NEURAL") {
        if (!canAfford(s, r, 0, 12)) return;
        s.energy -= 12;
        s.neural += 12;
        sayAll(r, LineKind::System, dialogueLines(c, "stabilize.neural"));
        spend(s, c, r, 45);
    } else if (arg == "ORGAN") {
        if (!canAfford(s, r, 5, 12)) return;
        s.energy -= 12;
        s.biomass -= 5;
        s.organ += 8;
        sayAll(r, LineKind::System, dialogueLines(c, "stabilize.organ"));
        spend(s, c, r, 45);
    } else {
        say(r, LineKind::Error, "STABILIZE needs a target: IMMUNE, NEURAL or ORGAN.");
    }
}

void evolve(GameState& s, const Content& c, const std::string& arg, CommandResult& r) {
    const BranchDef* branch = findBranch(c, arg);
    if (!branch) {
        say(r, LineKind::Error, "EVOLVE needs a branch: REGENERATION, CAMOUFLAGE or NEURAL INTERFACE.");
        return;
    }

    const int level = evolutionLevel(s, branch->stat);
    if (level >= static_cast<int>(branch->levels.size())) {
        say(r, LineKind::Error, branch->name + " is already at maximum level.");
        return;
    }
    const BranchLevel& next = branch->levels[static_cast<size_t>(level)];

    if (!canAfford(s, r, next.biomass, next.energy)) return;
    if (s.adaptation < next.adaptation) {
        say(r, LineKind::Error, "Insufficient adaptation: need " + numText(next.adaptation) +
                                    ", have " + numText(s.adaptation) + ".");
        return;
    }

    s.biomass -= next.biomass;
    s.energy -= next.energy;
    s.adaptation -= next.adaptation;
    raiseEvolution(s, branch->stat);
    s.flags.insert("evolved");

    say(r, LineKind::System, "EVOLUTION COMPLETE: " + branch->name + " level " + std::to_string(level + 1));
    say(r, LineKind::Narrative, next.text);
    spend(s, c, r, next.minutes);
}

void communicate(GameState& s, const Content& c, CommandResult& r) {
    if (s.energy < 5.0) {
        say(r, LineKind::Error, "Insufficient energy to search neural activity.");
        return;
    }
    s.energy -= 5.0;
    say(r, LineKind::System, "SEARCHING NEURAL ACTIVITY...");
    if (s.neural < 25.0) {
        s.vitality -= 3.0;
        sayAll(r, LineKind::Warning, dialogueLines(c, "communicate.overload"));
    }

    for (const auto& m : c.memories) {
        if (s.flags.count("mem_" + m.id)) continue;
        if (!evalCondition(m.requires_, s)) continue;

        const int confidence = std::min(95, m.confidence + 15 * evolutionLevel(s, "neural_interface"));
        s.flags.insert("mem_" + m.id);
        s.memoriesFound++;
        s.adaptation += 2.0;

        say(r, LineKind::Narrative, "FRAGMENT RECOVERED: " + m.ru);
        say(r, LineKind::Narrative, "Translation: " + m.en);
        say(r, LineKind::Info, "Translation confidence: " + std::to_string(confidence) + "%.");
        spend(s, c, r, 30);
        return;
    }

    sayAll(r, LineKind::Narrative, dialogueLines(c, "communicate.none"));
    spend(s, c, r, 30);
}

void wait(GameState& s, const Content& c, const std::string& arg, CommandResult& r) {
    int minutes = 60;
    if (!arg.empty()) {
        try {
            size_t used = 0;
            minutes = std::stoi(arg, &used);
            if (used != arg.size()) throw std::invalid_argument("trailing characters");
        } catch (...) {
            say(r, LineKind::Error, "WAIT expects a number of minutes between 10 and 1440.");
            return;
        }
    }
    if (minutes < 10 || minutes > 1440) {
        say(r, LineKind::Error, "WAIT expects a number of minutes between 10 and 1440.");
        return;
    }
    spend(s, c, r, minutes);
}

} // namespace

bool runTimedAction(GameState& s, const Content& c, const std::string& cmd, const std::string& arg,
                    CommandResult& r) {
    static const std::set<std::string> timed = {
        "DIAGNOSE", "HEAL", "ABSORB", "STABILIZE", "EVOLVE", "COMMUNICATE", "WAIT"};
    if (!timed.count(cmd)) return false;

    if (decisionPending(s)) {
        say(r, LineKind::Error, "A final decision is pending. Type DECIDE PROTECT, DECIDE CLAIM or DECIDE RELEASE.");
        return true;
    }

    if (cmd == "DIAGNOSE") diagnose(s, c, arg, r);
    else if (cmd == "HEAL") heal(s, c, arg, r);
    else if (cmd == "ABSORB") absorb(s, c, arg, r);
    else if (cmd == "STABILIZE") stabilize(s, c, arg, r);
    else if (cmd == "EVOLVE") evolve(s, c, arg, r);
    else if (cmd == "COMMUNICATE") communicate(s, c, r);
    else if (cmd == "WAIT") wait(s, c, arg, r);
    return true;
}

void runDecision(GameState& s, const Content& c, const std::string& arg, CommandResult& r) {
    if (!decisionPending(s)) {
        say(r, LineKind::Error, "No decision is pending.");
        return;
    }

    std::string key;
    if (arg == "PROTECT") {
        // Everything left in S.O.R.A. goes into the heart.
        s.organ += std::min(30.0, s.biomass * 1.5);
        s.vitality += 8.0;
        s.biomass = 0.0;
        s.flags.insert("choice_protect");
        key = "decide.protect";
    } else if (arg == "CLAIM") {
        s.integrity += 10.0;
        s.immunity += 10.0;
        s.flags.insert("choice_claim");
        key = "decide.claim";
    } else if (arg == "RELEASE") {
        s.integrity -= 15.0;
        s.immunity -= 10.0;
        s.flags.insert("choice_release");
        key = "decide.release";
    } else {
        say(r, LineKind::Error, "DECIDE expects PROTECT, CLAIM or RELEASE.");
        return;
    }

    s.flags.insert("decision_done");
    sayAll(r, LineKind::Narrative, dialogueLines(c, key));
    spend(s, c, r, TOTAL_MINUTES - s.minutes);
}
