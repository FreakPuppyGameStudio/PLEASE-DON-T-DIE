#include "simulation/simulation.h"

#include <algorithm>

#include "systems/evolution_system.h"

namespace {

double clampValue(double v, double lo, double hi) {
    return std::max(lo, std::min(hi, v));
}

} // namespace

void clampState(GameState& s) {
    s.vitality = clampValue(s.vitality, 0.0, 100.0);
    s.oxygenation = clampValue(s.oxygenation, 0.0, 100.0);
    s.immunity = clampValue(s.immunity, 0.0, 100.0);
    s.neural = clampValue(s.neural, 0.0, 100.0);
    s.organ = clampValue(s.organ, 0.0, 100.0);
    s.biomass = clampValue(s.biomass, 0.0, 100.0);
    s.energy = clampValue(s.energy, 0.0, 100.0);
    s.integrity = clampValue(s.integrity, 0.0, 100.0);
    s.adaptation = clampValue(s.adaptation, 0.0, 100.0);
    s.minutes = static_cast<int>(clampValue(s.minutes, 0.0, TOTAL_MINUTES));
    s.nutrientBags = std::max(0, s.nutrientBags);
    s.evoRegen = static_cast<int>(clampValue(s.evoRegen, 0, MAX_EVOLUTION));
    s.evoCamo = static_cast<int>(clampValue(s.evoCamo, 0, MAX_EVOLUTION));
    s.evoNeural = static_cast<int>(clampValue(s.evoNeural, 0, MAX_EVOLUTION));
}

void tickBiology(GameState& s, double h) {
    // Oxygenation: healthy organs sustain it, damaged ones starve the tissue.
    s.oxygenation += (s.organ >= 50.0 ? 0.5 : -1.0) * h;
    if (s.immunity >= 60.0) s.oxygenation -= 0.5 * h;

    // Immune rejection grows unless camouflage slows it.
    s.immunity += 0.9 * immuneGrowthFactor(s) * h;

    // Organs degrade under rejection; a healthy S.O.R.A. supports them.
    s.organ -= (0.4 + s.immunity / 80.0) * h;
    if (s.oxygenation < 40.0) s.organ -= 0.5 * h;
    if (s.integrity >= 50.0) s.organ += 0.2 * h;

    // Neural stability: the neural interface helps, low oxygen hurts.
    s.neural += (neuralGainPerHour(s) - (s.oxygenation < 50.0 ? 0.8 : 0.05)) * h;

    // Vitality follows oxygen, organs and neural stability.
    double vit = 0.0;
    if (s.oxygenation < 40.0) vit -= 2.0;
    else if (s.oxygenation < 60.0) vit -= 0.5;
    if (s.organ < 30.0) vit -= 2.0;
    else if (s.organ < 50.0) vit -= 0.6;
    if (s.neural < 20.0) vit -= 1.0;
    if (s.oxygenation >= 70.0 && s.organ >= 60.0) vit += 0.8;
    s.vitality += vit * h;

    // S.O.R.A. integrity: rejection wears it down, its own repair costs energy.
    if (s.immunity > 30.0) s.integrity -= (s.immunity - 30.0) / 30.0 * h;
    if (s.energy >= 5.0 && s.integrity < 100.0) {
        s.integrity += 0.4 * repairMultiplier(s) * h;
        s.energy -= 0.6 * h;
    }
    s.energy += 1.0 * h;

    clampState(s);
}
