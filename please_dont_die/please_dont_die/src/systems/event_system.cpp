#include "systems/event_system.h"

#include "systems/condition.h"

void checkEvents(GameState& s, const Content& c, std::vector<OutLine>& out) {
    for (const auto& ev : c.events) {
        if (s.fired.count(ev.id)) continue;
        if (!evalCondition(ev.when, s)) continue;
        s.fired.insert(ev.id);
        out.push_back({ev.kind, ev.text});
    }
}
