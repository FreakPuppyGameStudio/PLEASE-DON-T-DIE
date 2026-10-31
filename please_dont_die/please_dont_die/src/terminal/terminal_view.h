#pragma once
// Raylib front end for the terminal: text only, black screen, monospaced font.

#include <optional>
#include <string>
#include <vector>

#include "raylib.h"
#include "simulation/simulation.h"

constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 640;

class TerminalView {
public:
    void init();
    void shutdown();

    void push(LineKind kind, const std::string& text);
    void pushAll(const std::vector<OutLine>& lines);
    void clear();

    // Reads typed characters. Returns the submitted line when Enter is pressed.
    std::optional<std::string> pollInput();

    void draw(const GameState& state, bool musicMuted) const;

    const Font& font() const { return font_; }

private:
    struct Row {
        LineKind kind;
        std::string text;
    };

    float measure(const std::string& s) const;

    Font font_{};
    bool customFont_ = false;
    std::vector<Row> rows_;
    std::string input_;
};
