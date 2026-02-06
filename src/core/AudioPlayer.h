#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <string>
#include <atomic>
#include <thread>

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();
    
    void play(const std::string& sound_file, bool vibrate);
    void stop();
    bool isPlaying() const { return playing_; }
    
private:
    void playLoop();
    void triggerVibration();
    bool playBeep();
    bool isTermuxEnvironment();
    
    std::atomic<bool> playing_{false};
    std::thread play_thread_;
    bool use_vibration_{false};
    bool is_termux_{false};
};

#endif