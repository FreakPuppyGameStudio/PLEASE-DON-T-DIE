#include "systems/time_system.h"

#include <algorithm>
#include <cstdio>

#include "systems/ending_system.h"
#include "systems/event_system.h"

namespace {

int dayOf(const GameState& s) {
    return std::min(3, s.minutes / (24 * 60) + 1);
}

void openDecision(GameState& s, const Content& c, std::vector<OutLine>& out) {
    s.flags.insert("decision_open");
    for (const auto& l : dialogueLines(c, "decision.open")) out.push_back({LineKind::Warning, l});
    for (const char* key : {"decision.protect.desc", "decision.claim.desc", "decision.release.desc"}) {
        for (const auto& l : dialogueLines(c, key)) out.push_back({LineKind::Info, l});
    }
}

// Returns true if the clock must stop here for the player's decision.
bool holdForDecision(GameState& s, const Content& c, std::vector<OutLine>& out) {
    if (s.minutes < DECISION_MINUTE || s.flags.count("decision_done")) return false;
    if (!s.flags.count("decision_open")) openDecision(s, c, out);
    return true;
}

} // namespace

std::string clockText(const GameState& s) {
    const int day = dayOf(s);
    const int tod = s.minutes - (day - 1) * 24 * 60;
    char buf[64];
    std::snprintf(buf, sizeof buf, "DAY %d - %02d:%02d", day, tod / 60, tod % 60);
    return buf;
}

std::string remainingText(const GameState& s) {
    const int left = TOTAL_MINUTES - s.minutes;
    char buf[96];
    std::snprintf(buf, sizeof buf, "TIME REMAINING: %d HOURS %d MINUTES.", left / 60, left % 60);
    return buf;
}

bool decisionPending(const GameState& s) {
    return s.flags.count("decision_open") > 0 && s.flags.count("decision_done") == 0;
}

void advanceTime(GameState& s, int minutes, const Content& c, std::vector<OutLine>& out) {
    if (resolveEnding(s, c, out)) return;
    if (holdForDecision(s, c, out)) return;

    int remaining = minutes;
    while (remaining > 0) {
        int step = std::min(10, remaining);
        if (s.minutes < DECISION_MINUTE) step = std::min(step, DECISION_MINUTE - s.minutes);

        tickBiology(s, step / 60.0);
        s.minutes = std::min(TOTAL_MINUTES, s.minutes + step);
        remaining -= step;

        checkEvents(s, c, out);
        if (resolveEnding(s, c, out)) return;
        if (holdForDecision(s, c, out)) return;
    }
}
