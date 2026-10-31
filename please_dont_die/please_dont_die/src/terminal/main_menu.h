#pragma once
// Title menu in the same terminal style as the game.
// Raylib is used only for input and drawing; the menu holds no game state.

#include <string>
#include <vector>

#include "raylib.h"

enum class MenuAction { None, Resume, NewGame, LoadSave, Help, ToggleMusic, Exit };

class MainMenu {
public:
    // Call once per frame while the menu is visible. Returns the chosen action, if any.
    MenuAction update(bool canResume, bool hasSave, bool musicOn) {
        if (showHelp_) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) ||
                IsKeyPressed(KEY_BACKSPACE)) {
                showHelp_ = false;
            }
            return MenuAction::None;
        }

        const auto list = items(canResume, hasSave, musicOn);
        const int n = static_cast<int>(list.size());
        if (IsKeyPressed(KEY_DOWN)) selected_ = (selected_ + 1) % n;
        if (IsKeyPressed(KEY_UP)) selected_ = (selected_ + n - 1) % n;
        for (int i = 0; i < n; ++i) {
            if (IsKeyPressed(KEY_ONE + i)) selected_ = i;
        }

        if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_KP_ENTER)) return MenuAction::None;

        const Item& item = list[static_cast<size_t>(selected_)];
        if (!item.enabled) {
            addNotice(item.reason);
            return MenuAction::None;
        }
        clearNotices();
        if (item.action == MenuAction::Help) {
            showHelp_ = true;
            return MenuAction::None;
        }
        return item.action;
    }

    void draw(const Font& font, bool canResume, bool hasSave, bool musicOn) const {
        BeginDrawing();
        ClearBackground(BLACK);

        float y = 50.0f;
        const float x = 60.0f;
        auto write = [&](const std::string& text, Color color) {
            DrawTextEx(font, text.c_str(), Vector2{x, y}, 22.0f, 1.0f, color);
            y += 28.0f;
        };

        if (showHelp_) {
            write("HOW TO PLAY", kSystem);
            write(std::string(60, '='), kInfo);
            for (const char* line : kHelpLines) write(line, kNarrative);
            y += 14.0f;
            write("ESC or ENTER returns to the menu.", kInfo);
        } else {
            write("PLEASE, DON'T DIE", kSystem);
            write("S.O.R.A. BIOLOGICAL INTERFACE v0.1", kSystem);
            write(std::string(60, '='), kInfo);
            write("HOST: ANASTASIA VOLKOVA   |   LOCATION: INTENSIVE CARE UNIT", kInfo);
            y += 20.0f;

            const auto list = items(canResume, hasSave, musicOn);
            for (size_t i = 0; i < list.size(); ++i) {
                const bool selected = static_cast<int>(i) == selected_;
                const std::string prefix = selected ? "> " : "  ";
                const std::string label = std::to_string(i + 1) + ". " + list[i].label;
                const Color color = !list[i].enabled ? kDim : (selected ? kSystem : kNarrative);
                write(prefix + label, color);
            }

            y += 20.0f;
            for (const auto& n : notices_) write(n, kWarning);
            y += 10.0f;
            write("UP/DOWN select   ENTER confirm   1-6 choose", kInfo);
        }

        EndDrawing();
    }

    void open() { showHelp_ = false; }
    void addNotice(const std::string& text) { notices_.push_back(text); }
    void clearNotices() { notices_.clear(); }

private:
    struct Item {
        MenuAction action;
        std::string label;
        bool enabled;
        std::string reason;  // shown when the item is chosen but unavailable
    };

    static std::vector<Item> items(bool canResume, bool hasSave, bool musicOn) {
        return {
            {MenuAction::Resume, "RESUME", canResume, "No game in progress."},
            {MenuAction::NewGame, "NEW GAME", true, ""},
            {MenuAction::LoadSave, "LOAD SAVED GAME", hasSave, "No saved game found. Use SAVE during a game first."},
            {MenuAction::Help, "HOW TO PLAY", true, ""},
            {MenuAction::ToggleMusic, musicOn ? "MUSIC: ON" : "MUSIC: OFF", true, ""},
            {MenuAction::Exit, "EXIT", true, ""},
        };
    }

    inline static const Color kSystem{0, 255, 120, 255};
    inline static const Color kNarrative{255, 190, 80, 255};
    inline static const Color kWarning{255, 110, 60, 255};
    inline static const Color kInfo{0, 170, 90, 255};
    inline static const Color kDim{90, 90, 90, 255};

    inline static const char* const kHelpLines[] = {
        "You are S.O.R.A., an organism living inside Anastasia Volkova.",
        "Keep her alive and keep yourself alive for 72 hours.",
        "Type commands at the S.O.R.A.> prompt and press ENTER.",
        "Time passes only when you act. STATUS and HELP cost no time.",
        "Read the warnings. Every choice spends biomass, energy or time.",
        "At hour 66 the clock stops: DECIDE PROTECT, CLAIM or RELEASE.",
        "Type HELP in the game for the full command list.",
        "ESC in the game returns to this menu. F2 toggles the music.",
    };

    int selected_ = 0;
    bool showHelp_ = false;
    std::vector<std::string> notices_;
};
