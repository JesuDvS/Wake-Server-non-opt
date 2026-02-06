#include "AudioPlayer.h"
#include "../utils/Logger.h"
#include <cstdlib>
#include <iostream>
#include <chrono>

AudioPlayer::AudioPlayer() {}

AudioPlayer::~AudioPlayer() {
    stop();
}

void AudioPlayer::play(const std::string& sound_file, bool vibrate) {
    if (playing_) {
        stop();
    }
    
    playing_ = true;
    use_vibration_ = vibrate;
    
    play_thread_ = std::thread(&AudioPlayer::playLoop, this);
}

void AudioPlayer::stop() {
    playing_ = false;
    if (play_thread_.joinable()) {
        play_thread_.join();
    }
}

void AudioPlayer::playLoop() {
    Logger::info("Reproduciendo alarma...");
    
    int beep_count = 0;
    
    while (playing_) {
        // Intentar reproducir sonido
        if (!playBeep()) {
            // Fallback: imprimir alerta visual en consola
            std::cout << "\n\n🔔🔔🔔 ¡ALARMA! 🔔🔔🔔\n" << std::flush;
            std::cout << "⏰ Presiona el botón en la interfaz web para detener\n\n" << std::flush;
        }
        
        // Vibración (si termux-vibrate está disponible)
        if (use_vibration_ && beep_count % 2 == 0) {
            triggerVibration();
        }
        
        beep_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
    
    Logger::info("Reproducción detenida");
}

bool AudioPlayer::playBeep() {
    // Método 1: Usar termux-media-player
    int result = system("termux-media-player play /system/media/audio/alarms/Argon.ogg 2>/dev/null");
    if (result == 0) return true;
    
    // Método 2: Usar beep del sistema
    result = system("tput bel 2>/dev/null");
    if (result == 0) return true;
    
    // Método 3: printf con caracteres de alarma
    std::cout << "\a" << std::flush;
    
    return false;
}

void AudioPlayer::triggerVibration() {
    system("termux-vibrate -d 500 2>/dev/null");
}