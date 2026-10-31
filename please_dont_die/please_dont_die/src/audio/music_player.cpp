#include "audio/music_player.h"

bool MusicPlayer::init(const std::string& path, std::string& status) {
    InitAudioDevice();
    deviceOpen_ = IsAudioDeviceReady();
    if (!deviceOpen_) {
        status = "AUDIO: no audio device found. Running silent.";
        return false;
    }
    if (!FileExists(path.c_str())) {
        status = "AUDIO: " + path + " not found. Running silent.";
        return false;
    }
    music_ = LoadMusicStream(path.c_str());
    ready_ = music_.frameCount > 0;  // 0 means the file could not be loaded
    if (!ready_) {
        status = "AUDIO: could not decode " + path + ". Running silent.";
        return false;
    }
    SetMusicVolume(music_, 0.45f);
    PlayMusicStream(music_);
    return true;
}

void MusicPlayer::update() {
    if (!ready_) return;
    UpdateMusicStream(music_);
    // Raylib streams do not loop by themselves: restart when playback ends.
    if (!muted_ && !IsMusicStreamPlaying(music_)) {
        SeekMusicStream(music_, 0.0f);
        PlayMusicStream(music_);
    }
}

void MusicPlayer::toggle() {
    muted_ = !muted_;
    if (!ready_) return;
    if (muted_) PauseMusicStream(music_);
    else ResumeMusicStream(music_);
}

void MusicPlayer::shutdown() {
    if (ready_) {
        UnloadMusicStream(music_);
        ready_ = false;
    }
    if (deviceOpen_) {
        CloseAudioDevice();
        deviceOpen_ = false;
    }
}
