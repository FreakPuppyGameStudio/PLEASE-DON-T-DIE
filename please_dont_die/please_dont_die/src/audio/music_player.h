#pragma once
// Background music: loops audio/gm5.ogg. If anything is missing, the game runs silent.

#include <string>

#include "raylib.h"

class MusicPlayer {
public:
    MusicPlayer() = default;
    ~MusicPlayer() { shutdown(); }
    MusicPlayer(const MusicPlayer&) = delete;
    MusicPlayer& operator=(const MusicPlayer&) = delete;

    // Returns true if music is playing. On failure, `status` explains why.
    bool init(const std::string& path, std::string& status);
    void update();  // call once per frame; restarts the track when it ends
    void toggle();
    bool muted() const { return muted_; }
    void shutdown();

private:
    Music music_{};
    bool ready_ = false;
    bool deviceOpen_ = false;
    bool muted_ = false;
};
