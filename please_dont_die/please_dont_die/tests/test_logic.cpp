// Headless tests: no Raylib needed.
// Build from the project root:
//   g++ -std=c++17 -Isrc tests/test_logic.cpp $(find src -name '*.cpp' ! -name main.cpp ! -path '*/audio/*' ! -path '*/terminal/terminal_view.cpp') -o test_logic

#include <cstdio>
#include <string>
#include <vector>

#include "narrative/content.h"
#include "narrative/json.h"
#include "save/save_system.h"
#include "simulation/simulation.h"
#include "systems/condition.h"
#include "systems/time_system.h"
#include "terminal/commands.h"
#include "terminal/text_wrap.h"

static int failures = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
            ++failures;                                                   \
        }                                                                 \
    } while (0)

static Content gContent;

static bool hasKind(const CommandResult& r, LineKind k) {
    for (const auto& l : r.lines)
        if (l.kind == k) return true;
    return false;
}

static bool hasText(const CommandResult& r, const std::string& needle) {
    for (const auto& l : r.lines)
        if (l.text.find(needle) != std::string::npos) return true;
    return false;
}

// --- Parser for data files ---------------------------------------------------

static void testJsonParser() {
    mini_json::Value v;
    std::string err;
    CHECK(mini_json::parse(R"({"a": [1, 2.5, -3e2], "b": "x\"y\u0041", "c": true, "d": null})", v, err));
    const auto* a = v.find("a");
    CHECK(a && a->items.size() == 3);
    CHECK(a && a->items[2].number == -300.0);
    CHECK(v.stringOr("b", "") == "x\"yA");
    CHECK(mini_json::parse("{\"ru\": \"мама\"}", v, err));
    CHECK(v.stringOr("ru", "") == "мама");
    CHECK(!mini_json::parse("{\"a\": }", v, err));
    CHECK(!mini_json::parse("[1, 2", v, err));
}

static void testContentLoads() {
    std::string err;
    CHECK(loadContent("data", gContent, err));
    if (!err.empty()) std::printf("  content error: %s\n", err.c_str());
    CHECK(gContent.events.size() >= 10);
    CHECK(gContent.memories.size() == 6);
    CHECK(gContent.branches.size() == 3);
    CHECK(!dialogueLines(gContent, "ending.SYMBIOSIS").empty());
    CHECK(!dialogueLines(gContent, "decision.open").empty());
}

// --- Conditions and text -------------------------------------------------------

static void testConditions() {
    GameState s;
    s.minutes = 600;
    s.immunity = 45;
    s.flags.insert("evolved");
    CHECK(evalCondition("", s));
    CHECK(evalCondition("minutes>=600", s));
    CHECK(!evalCondition("minutes>600", s));
    CHECK(evalCondition("immunity>=40 and flag:evolved", s));
    CHECK(!evalCondition("immunity>=40 and flag:mem_mom", s));
    CHECK(evalCondition("!flag:mem_mom", s));
    CHECK(!evalCondition("unknownstat>=1", s));
}

static void testWrap() {
    auto measure = [](const std::string& s) { return static_cast<float>(s.size()) * 10.0f; };
    auto lines = wrapText("one two three four", 100.0f, measure);
    CHECK(lines.size() == 2);
    CHECK(lines.size() >= 1 && lines[0] == "one two");
    CHECK(wrapText("", 100.0f, measure).size() == 1);
}

// --- Commands and time -----------------------------------------------------------

static void testStatusIsFree() {
    GameState s;
    executeCommand(s, gContent, "STATUS");
    executeCommand(s, gContent, "INVENTORY");
    executeCommand(s, gContent, "HELP");
    CHECK(s.minutes == 0);
}

static void testUnknownCommand() {
    GameState s;
    CommandResult r = executeCommand(s, gContent, "DANCE");
    CHECK(hasKind(r, LineKind::Error));
    CHECK(s.minutes == 0);
}

static void testCaseInsensitive() {
    GameState s;
    executeCommand(s, gContent, "wait 30");
    CHECK(s.minutes == 30);
}

static void testHealCostsAndEffects() {
    GameState s;
    const double biomassBefore = s.biomass;
    executeCommand(s, gContent, "HEAL ORGAN");
    CHECK(s.minutes == 60);
    CHECK(s.biomass < biomassBefore);
    GameState control;
    executeCommand(control, gContent, "WAIT 60");
    CHECK(s.organ > control.organ);
}

static void testHealRejectedWithoutBiomass() {
    GameState s;
    s.biomass = 0;
    CHECK(hasKind(executeCommand(s, gContent, "HEAL ORGAN"), LineKind::Error));
    CHECK(s.minutes == 0);
}

static void testTissueHarmsHost() {
    GameState s;
    const double vit = s.vitality;
    executeCommand(s, gContent, "ABSORB TISSUE");
    CHECK(s.vitality < vit);
    CHECK(s.tissueAbsorbed == 1);
    CHECK(s.minutes == 45);
}

static void testNutrientBagsRunOut() {
    GameState s;
    for (int i = 0; i < 4; ++i) executeCommand(s, gContent, "ABSORB NUTRIENTS");
    CHECK(hasKind(executeCommand(s, gContent, "ABSORB NUTRIENTS"), LineKind::Error));
    CHECK(s.nutrientBags == 0);
}

static void testEvolveLimitsAndCosts() {
    GameState s;
    s.biomass = 500;
    s.energy = 500;
    s.adaptation = 100;
    executeCommand(s, gContent, "EVOLVE CAMOUFLAGE");
    CHECK(s.evoCamo == 1);
    CHECK(s.minutes == 120);
    executeCommand(s, gContent, "EVOLVE CAMOUFLAGE");
    CHECK(s.evoCamo == 2);
    CHECK(hasKind(executeCommand(s, gContent, "EVOLVE CAMOUFLAGE"), LineKind::Error));
    CHECK(s.evoCamo == 2);
}

static void testEvolveNeedsAdaptation() {
    GameState s;
    s.adaptation = 0;
    CHECK(hasKind(executeCommand(s, gContent, "EVOLVE REGENERATION"), LineKind::Error));
    CHECK(s.evoRegen == 0);
}

static void testNeuralInterfaceBranchParsing() {
    GameState s;
    s.biomass = 100;
    s.energy = 100;
    s.adaptation = 100;
    executeCommand(s, gContent, "EVOLVE NEURAL INTERFACE");
    CHECK(s.evoNeural == 1);
}

static void testCommunicateRevealsMemories() {
    GameState s;
    s.neural = 60;
    executeCommand(s, gContent, "COMMUNICATE");
    CHECK(s.memoriesFound == 1);
    CHECK(s.flags.count("mem_mom") == 1);
}

static void testWaitValidation() {
    GameState s;
    CHECK(hasKind(executeCommand(s, gContent, "WAIT 5"), LineKind::Error));
    CHECK(hasKind(executeCommand(s, gContent, "WAIT abc"), LineKind::Error));
    CHECK(hasKind(executeCommand(s, gContent, "WAIT 5000"), LineKind::Error));
    CHECK(s.minutes == 0);
}

static void testClock() {
    GameState s;
    s.minutes = 30;
    CHECK(clockText(s) == "DAY 1 - 00:30");
    s.minutes = 1440 + 90;
    CHECK(clockText(s) == "DAY 2 - 01:30");
    CHECK(remainingText(s) == "TIME REMAINING: 46 HOURS 30 MINUTES.");
}

// --- Final decision and endings ------------------------------------------------

static void testClockHoldsForDecision() {
    GameState s;
    s.minutes = 3950;
    executeCommand(s, gContent, "WAIT 60");
    CHECK(s.minutes == DECISION_MINUTE);
    CHECK(decisionPending(s));
    CHECK(hasKind(executeCommand(s, gContent, "WAIT 10"), LineKind::Error));
    CHECK(s.minutes == DECISION_MINUTE);
    CHECK(hasKind(executeCommand(s, gContent, "DECIDE MAYBE"), LineKind::Error));
}

static void testDecideWithoutPendingFails() {
    GameState s;
    CHECK(hasKind(executeCommand(s, gContent, "DECIDE PROTECT"), LineKind::Error));
}

static void testFullRunEndsAfterDecision() {
    GameState s;
    CommandResult r = executeCommand(s, gContent, "WAIT 1440");
    CHECK(!hasKind(r, LineKind::Error));
    executeCommand(s, gContent, "WAIT 1440");
    executeCommand(s, gContent, "WAIT 1440");
    CHECK(decisionPending(s));
    CHECK(!s.over);
    executeCommand(s, gContent, "DECIDE RELEASE");
    CHECK(s.over);
    CHECK(s.minutes <= TOTAL_MINUTES);
    CHECK(!s.ending.empty());
}

static void testRecoveryEnding() {
    GameState s;
    s.vitality = 90;
    s.neural = 90;
    s.memoriesFound = 3;
    s.evoNeural = 1;
    s.organ = 90;
    s.oxygenation = 90;
    s.immunity = 10;
    s.integrity = 90;
    s.minutes = DECISION_MINUTE;
    std::vector<OutLine> opened;
    advanceTime(s, 0, gContent, opened);  // opens the decision window
    executeCommand(s, gContent, "DECIDE PROTECT");
    CHECK(s.over);
    CHECK(s.ending == "RECOVERY");
}

static void testReleaseBlocksRecovery() {
    GameState s;
    s.vitality = 90;
    s.neural = 90;
    s.memoriesFound = 3;
    s.evoNeural = 1;
    s.organ = 90;
    s.oxygenation = 90;
    s.immunity = 10;
    s.integrity = 90;
    s.minutes = DECISION_MINUTE;
    std::vector<OutLine> opened;
    advanceTime(s, 0, gContent, opened);
    executeCommand(s, gContent, "DECIDE RELEASE");
    CHECK(s.ending == "SYMBIOSIS");
}

static void testClaimGivesTakeover() {
    GameState s;
    s.minutes = DECISION_MINUTE;
    s.integrity = 50;
    std::vector<OutLine> opened;
    advanceTime(s, 0, gContent, opened);
    executeCommand(s, gContent, "DECIDE CLAIM");
    CHECK(s.ending == "TAKEOVER");
}

static void testTerminationWhenBothDie() {
    GameState s;
    s.vitality = 0;
    s.integrity = 0;
    std::vector<OutLine> out;
    advanceTime(s, 10, gContent, out);
    CHECK(s.over);
    CHECK(s.ending == "TERMINATION");
}

static void testSacrificeWhenParasiteDies() {
    GameState s;
    s.integrity = 0;
    s.energy = 0;
    std::vector<OutLine> out;
    advanceTime(s, 10, gContent, out);
    CHECK(s.ending == "SACRIFICE");
}

static void testTakeoverWhenHostDies() {
    GameState s;
    s.vitality = 0;
    std::vector<OutLine> out;
    advanceTime(s, 10, gContent, out);
    CHECK(s.ending == "TAKEOVER");
}

static void testEndedSessionRejectsActions() {
    GameState s;
    s.vitality = 0;
    std::vector<OutLine> out;
    advanceTime(s, 10, gContent, out);
    CHECK(hasKind(executeCommand(s, gContent, "WAIT 60"), LineKind::Error));
    CHECK(hasText(executeCommand(s, gContent, "STATUS"), "DAY"));
}

// --- Save and load -------------------------------------------------------------

static void testSaveLoadRoundTrip() {
    GameState s;
    s.vitality = 61.5;
    s.biomass = 22.25;
    s.evoCamo = 1;
    s.memoriesFound = 2;
    s.minutes = 1234;
    s.flags.insert("mem_mom");
    s.fired.insert("day2");

    const std::string path = "saves/test_roundtrip.txt";
    std::string err;
    CHECK(saveGame(s, path, err));

    GameState t;
    CHECK(loadGame(t, path, err));
    CHECK(t.vitality > 61.49 && t.vitality < 61.51);
    CHECK(t.biomass > 22.24 && t.biomass < 22.26);
    CHECK(t.evoCamo == 1);
    CHECK(t.memoriesFound == 2);
    CHECK(t.minutes == 1234);
    CHECK(t.flags.count("mem_mom") == 1);
    CHECK(t.fired.count("day2") == 1);
    CHECK(!t.over);
    std::remove(path.c_str());
}

static void testLoadMissingFile() {
    GameState s;
    std::string err;
    CHECK(!loadGame(s, "saves/does_not_exist.txt", err));
    CHECK(!err.empty());
}

int main() {
    testJsonParser();
    testContentLoads();
    testConditions();
    testWrap();
    testStatusIsFree();
    testUnknownCommand();
    testCaseInsensitive();
    testHealCostsAndEffects();
    testHealRejectedWithoutBiomass();
    testTissueHarmsHost();
    testNutrientBagsRunOut();
    testEvolveLimitsAndCosts();
    testEvolveNeedsAdaptation();
    testNeuralInterfaceBranchParsing();
    testCommunicateRevealsMemories();
    testWaitValidation();
    testClock();
    testClockHoldsForDecision();
    testDecideWithoutPendingFails();
    testFullRunEndsAfterDecision();
    testRecoveryEnding();
    testReleaseBlocksRecovery();
    testClaimGivesTakeover();
    testTerminationWhenBothDie();
    testSacrificeWhenParasiteDies();
    testTakeoverWhenHostDies();
    testEndedSessionRejectsActions();
    testSaveLoadRoundTrip();
    testLoadMissingFile();

    if (failures == 0) std::printf("All tests passed.\n");
    else std::printf("%d check(s) failed.\n", failures);
    return failures == 0 ? 0 : 1;
}
