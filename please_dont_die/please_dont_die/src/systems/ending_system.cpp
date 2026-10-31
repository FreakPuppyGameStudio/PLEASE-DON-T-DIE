#include "systems/ending_system.h"

#include "systems/evolution_system.h"

bool resolveEnding(GameState& s, const Content& c, std::vector<OutLine>& out) {
    if (s.over) return true;

    const bool hostDead = s.vitality <= 0.0;
    const bool parasiteDead = s.integrity <= 0.0;
    const bool timeUp = s.minutes >= TOTAL_MINUTES;
    if (!hostDead && !parasiteDead && !timeUp) return false;

    const bool claimed = s.flags.count("choice_claim") > 0;
    const bool released = s.flags.count("choice_release") > 0;

    // Order matters: deaths first, then the final-decision outcomes, then the rest.
    if (hostDead && parasiteDead) s.ending = "TERMINATION";
    else if (parasiteDead) s.ending = "SACRIFICE";
    else if (hostDead) s.ending = "TAKEOVER";
    else if (claimed && s.integrity >= 40.0) s.ending = "TAKEOVER";
    else if (!released && s.vitality >= 60.0 && s.neural >= 70.0 &&
             s.memoriesFound >= 3 && evolutionLevel(s, "neural_interface") >= 1) s.ending = "RECOVERY";
    else s.ending = "SYMBIOSIS";

    s.over = true;
    out.push_back({LineKind::System, "=== ENDING: " + s.ending + " ==="});
    for (const auto& line : dialogueLines(c, "ending." + s.ending)) out.push_back({LineKind::Narrative, line});
    out.push_back({LineKind::Info, "Type STATUS to review the final readings, or EXIT to close."});
    return true;
}
