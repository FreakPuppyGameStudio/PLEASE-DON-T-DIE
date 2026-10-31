#include "terminal/terminal_view.h"

#include "terminal/text_wrap.h"
#include "systems/time_system.h"

namespace {

constexpr int FONT_SIZE = 22;
constexpr float LINE_H = 26.0f;
constexpr float MARGIN = 20.0f;
constexpr size_t MAX_INPUT = 120;
constexpr size_t MAX_ROWS = 2000;

Color colorFor(LineKind k) {
    switch (k) {
        case LineKind::System:    return Color{0, 255, 120, 255};
        case LineKind::Narrative: return Color{255, 190, 80, 255};
        case LineKind::Warning:   return Color{255, 110, 60, 255};
        case LineKind::Error:     return Color{255, 60, 60, 255};
        case LineKind::Player:    return Color{230, 255, 230, 255};
        case LineKind::Info:      return Color{0, 170, 90, 255};
    }
    return WHITE;
}

} // namespace

void TerminalView::init() {
    // Cyrillic and ASCII glyphs are needed for the Russian fragments.
    // consola.ttf ships with Windows and covers both.
    std::vector<int> codepoints;
    for (int c = 32; c < 127; ++c) codepoints.push_back(c);
    for (int c = 0x0400; c <= 0x04FF; ++c) codepoints.push_back(c);

    const char* candidates[] = {"assets/terminal.ttf", "C:/Windows/Fonts/consola.ttf", "C:/Windows/Fonts/lucon.ttf"};
    for (const char* path : candidates) {
        if (!FileExists(path)) continue;
        Font f = LoadFontEx(path, FONT_SIZE, codepoints.data(), static_cast<int>(codepoints.size()));
        if (f.texture.id != 0) {
            font_ = f;
            customFont_ = true;
            break;
        }
    }
    if (!customFont_) {
        font_ = GetFontDefault();
        push(LineKind::Warning, "DISPLAY: monospaced font not found. Russian text may not render.");
    }
    SetTextureFilter(font_.texture, TEXTURE_FILTER_BILINEAR);
}

void TerminalView::shutdown() {
    if (customFont_) UnloadFont(font_);
    customFont_ = false;
}

float TerminalView::measure(const std::string& s) const {
    return MeasureTextEx(font_, s.c_str(), FONT_SIZE, 1.0f).x;
}

void TerminalView::push(LineKind kind, const std::string& text) {
    const float maxWidth = WINDOW_WIDTH - 2 * MARGIN;
    for (const auto& line : wrapText(text, maxWidth, [this](const std::string& s) { return measure(s); })) {
        rows_.push_back({kind, line});
    }
    if (rows_.size() > MAX_ROWS) {
        rows_.erase(rows_.begin(), rows_.begin() + static_cast<long>(rows_.size() - MAX_ROWS));
    }
}

void TerminalView::pushAll(const std::vector<OutLine>& lines) {
    for (const auto& l : lines) push(l.kind, l.text);
}

void TerminalView::clear() {
    rows_.clear();
}

std::optional<std::string> TerminalView::pollInput() {
    int ch;
    while ((ch = GetCharPressed()) != 0) {
        if (ch >= 32 && ch <= 126 && input_.size() < MAX_INPUT) input_.push_back(static_cast<char>(ch));
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !input_.empty()) input_.pop_back();
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        std::string submitted = input_;
        input_.clear();
        if (submitted.find_first_not_of(" \t") == std::string::npos) return std::nullopt;
        return submitted;
    }
    return std::nullopt;
}

void TerminalView::draw(const GameState& state, bool musicMuted) const {
    BeginDrawing();
    ClearBackground(BLACK);

    const std::string header = clockText(state) + "   |   " + remainingText(state) +
                               (musicMuted ? "   [MUSIC OFF]" : "");
    DrawTextEx(font_, header.c_str(), Vector2{MARGIN, MARGIN}, FONT_SIZE, 1.0f, colorFor(LineKind::System));
    DrawTextEx(font_, std::string(72, '=').c_str(), Vector2{MARGIN, MARGIN + LINE_H}, FONT_SIZE, 1.0f,
               colorFor(LineKind::Info));

    // Scrollback: the most recent rows sit just above the input line.
    const float inputY = WINDOW_HEIGHT - MARGIN - LINE_H;
    const float top = MARGIN + 2 * LINE_H;
    const int visible = static_cast<int>((inputY - top) / LINE_H);
    const int first = static_cast<int>(rows_.size()) - visible;
    for (int i = 0; i < visible; ++i) {
        const int idx = first + i;
        if (idx < 0) continue;
        const Row& row = rows_[static_cast<size_t>(idx)];
        DrawTextEx(font_, row.text.c_str(), Vector2{MARGIN, top + i * LINE_H}, FONT_SIZE, 1.0f, colorFor(row.kind));
    }

    const bool cursorOn = static_cast<int>(GetTime() * 2.0) % 2 == 0;
    const std::string prompt = "S.O.R.A.> " + input_ + (cursorOn ? "_" : " ");
    DrawTextEx(font_, prompt.c_str(), Vector2{MARGIN, inputY}, FONT_SIZE, 1.0f, colorFor(LineKind::Player));

    EndDrawing();
}
