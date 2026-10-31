#include "narrative/content.h"

#include <fstream>
#include <sstream>

#include "narrative/json.h"

namespace {

bool readJson(const std::string& path, mini_json::Value& root, std::string& err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        err = "missing file " + path + ".";
        return false;
    }
    std::stringstream ss;
    ss << f.rdbuf();
    std::string perr;
    if (!mini_json::parse(ss.str(), root, perr)) {
        err = path + ": " + perr + ".";
        return false;
    }
    return true;
}

LineKind kindFrom(const std::string& s) {
    if (s == "system") return LineKind::System;
    if (s == "warning") return LineKind::Warning;
    if (s == "error") return LineKind::Error;
    if (s == "info") return LineKind::Info;
    return LineKind::Narrative;
}

const mini_json::Value* arrayField(const mini_json::Value& root, const std::string& key) {
    const auto* v = root.find(key);
    return (v && v->type == mini_json::Value::Type::Array) ? v : nullptr;
}

} // namespace

bool loadContent(const std::string& dir, Content& out, std::string& err) {
    mini_json::Value root;

    if (!readJson(dir + "/events.json", root, err)) return false;
    const auto* events = arrayField(root, "events");
    if (!events) {
        err = "events.json has no 'events' array.";
        return false;
    }
    for (const auto& e : events->items) {
        EventDef d;
        d.id = e.stringOr("id", "");
        d.when = e.stringOr("when", "");
        d.kind = kindFrom(e.stringOr("kind", "narrative"));
        d.text = e.stringOr("text", "");
        if (d.id.empty()) {
            err = "events.json: an event has no id.";
            return false;
        }
        out.events.push_back(d);
    }

    if (!readJson(dir + "/memories.json", root, err)) return false;
    const auto* memories = arrayField(root, "memories");
    if (!memories) {
        err = "memories.json has no 'memories' array.";
        return false;
    }
    for (const auto& m : memories->items) {
        MemoryDef d;
        d.id = m.stringOr("id", "");
        d.ru = m.stringOr("ru", "");
        d.en = m.stringOr("en", "");
        d.confidence = static_cast<int>(m.numberOr("confidence", 50));
        d.requires_ = m.stringOr("requires", "");
        if (d.id.empty()) {
            err = "memories.json: a memory has no id.";
            return false;
        }
        out.memories.push_back(d);
    }

    if (!readJson(dir + "/upgrades.json", root, err)) return false;
    const auto* branches = arrayField(root, "branches");
    if (!branches) {
        err = "upgrades.json has no 'branches' array.";
        return false;
    }
    for (const auto& b : branches->items) {
        BranchDef d;
        d.id = b.stringOr("id", "");
        d.name = b.stringOr("name", "");
        d.stat = b.stringOr("stat", "");
        if (const auto* levels = arrayField(b, "levels")) {
            for (const auto& lv : levels->items) {
                BranchLevel l;
                l.biomass = lv.numberOr("biomass", 0);
                l.energy = lv.numberOr("energy", 0);
                l.adaptation = lv.numberOr("adaptation", 0);
                l.minutes = static_cast<int>(lv.numberOr("minutes", 120));
                l.text = lv.stringOr("text", "");
                d.levels.push_back(l);
            }
        }
        out.branches.push_back(d);
    }

    if (!readJson(dir + "/dialogue.json", root, err)) return false;
    if (const auto* lines = root.find("lines")) {
        for (const auto& member : lines->members) {
            std::vector<std::string> values;
            if (member.second.type == mini_json::Value::Type::String) {
                values.push_back(member.second.str);
            } else if (member.second.type == mini_json::Value::Type::Array) {
                for (const auto& item : member.second.items) {
                    if (item.type == mini_json::Value::Type::String) values.push_back(item.str);
                }
            }
            out.dialogue[member.first] = values;
        }
    }
    return true;
}

std::vector<std::string> dialogueLines(const Content& c, const std::string& key) {
    auto it = c.dialogue.find(key);
    return it == c.dialogue.end() ? std::vector<std::string>{} : it->second;
}
