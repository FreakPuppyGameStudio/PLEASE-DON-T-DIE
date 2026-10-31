// PLEASE, DON'T DIE v0.1: entry point. Title menu, game screen and music.

#include "audio/music_player.h"
#include "narrative/content.h"
#include "raylib.h"
#include "save/save_system.h"
#include "systems/time_system.h"
#include "terminal/commands.h"
#include "terminal/main_menu.h"
#include "terminal/terminal_view.h"

namespace {
enum class Screen { Menu, Game };
} // namespace

int main() {
    // data/, audio/ and saves/ are resolved next to the executable.
    ChangeDirectory(GetApplicationDirectory());

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PLEASE, DON'T DIE v0.1");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);  // ESC is used to return to the menu.

    TerminalView view;
    view.init();

    MainMenu menu;

    Content content;
    std::string err;
    if (!loadContent("data", content, err)) {
        menu.addNotice("DATA: " + err + " Continuing with reduced content.");
    }

    MusicPlayer music;
    std::string audioStatus;
    if (!music.init("audio/gm5.ogg", audioStatus)) menu.addNotice(audioStatus);

    GameState state;
    Screen screen = Screen::Menu;
    bool sessionActive = false;
    bool running = true;

    while (running && !WindowShouldClose()) {
        if (IsKeyPressed(KEY_F2)) music.toggle();  // works on both screens

        if (screen == Screen::Menu) {
            const bool hasSave = FileExists(SAVE_PATH.c_str());
            switch (menu.update(sessionActive, hasSave, !music.muted())) {
                case MenuAction::Resume:
                    screen = Screen::Game;
                    break;

                case MenuAction::NewGame:
                    state = GameState{};
                    view.clear();
                    view.pushAll(introLines(state));
                    sessionActive = true;
                    screen = Screen::Game;
                    break;

                case MenuAction::LoadSave: {
                    std::string loadErr;
                    if (loadGame(state, SAVE_PATH, loadErr)) {
                        view.clear();
                        view.push(LineKind::System, "Game loaded from " + SAVE_PATH + ".");
                        view.push(LineKind::System, clockText(state));
                        sessionActive = true;
                        screen = Screen::Game;
                    } else {
                        menu.addNotice("LOAD failed: " + loadErr);
                    }
                    break;
                }

                case MenuAction::ToggleMusic:
                    music.toggle();
                    break;

                case MenuAction::Exit:
                    running = false;
                    break;

                default:
                    break;
            }

            music.update();
            menu.draw(view.font(), sessionActive, hasSave, !music.muted());
        } else {
            if (IsKeyPressed(KEY_ESCAPE)) {
                screen = Screen::Menu;
                menu.open();
            } else if (auto line = view.pollInput()) {
                view.push(LineKind::Player, "S.O.R.A.> " + *line);
                CommandResult result = executeCommand(state, content, *line);
                if (result.clearScreen) view.clear();
                else view.pushAll(result.lines);
                if (result.exitRequested) running = false;
            }

            music.update();
            view.draw(state, music.muted());
        }
    }

    music.shutdown();
    view.shutdown();
    CloseWindow();
    return 0;
}
