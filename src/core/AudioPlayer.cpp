#include "AudioPlayer.h"
#include "../utils/Logger.h"
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <fstream>

AudioPlayer::AudioPlayer() {
    is_termux_ = isTermuxEnvironment();
    if (is_termux_) {
        Logger::info("✅ Entorno Termux detectado - Sonido habilitado");
    } else {
        Logger::warning("⚠️  No se detectó Termux - Sonido deshabilitado");
    }
}

AudioPlayer::~AudioPlayer() {
    stop();
}

bool AudioPlayer::isTermuxEnvironment() {
    // Verificar si estamos en Termux
    // 1. Verificar variable de entorno TERMUX_VERSION
    if (std::getenv("TERMUX_VERSION") != nullptr) {
        return true;
    }
    
    // 2. Verificar si existe el directorio de Termux
    std::ifstream termux_check("/data/data/com.termux/files/usr/bin/termux-info");
    if (termux_check.good()) {
        termux_check.close();
        return true;
    }
    
    // 3. Verificar si termux-api está disponible
    int result = system("command -v termux-vibrate >/dev/null 2>&1");
    if (result == 0) {
        return true;
    }
    
    return false;
}

void AudioPlayer::play(const std::string& sound_file, bool vibrate) {
    if (!is_termux_) {
        Logger::info("🔇 Alarma activada (sin sonido - no Termux)");
        return;
    }
    
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
    Logger::info("🔊 Reproduciendo alarma en el servidor (Termux)...");
    
    int beep_count = 0;
    
    while (playing_) {
        // Intentar reproducir sonido usando Termux API
        if (!playBeep()) {
            // Fallback: alerta visual en consola
            std::cout << "\n\n🔔🔔🔔 ¡ALARMA SONANDO! 🔔🔔🔔\n" << std::flush;
        }
        
        // Vibración (si termux-vibrate está disponible)
        if (use_vibration_ && beep_count % 2 == 0) {
            triggerVibration();
        }
        
        beep_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
    
    Logger::info("🔇 Reproducción detenida");
}

bool AudioPlayer::playBeep() {
    // Método 1: Usar termux-media-player con archivo de sistema
    int result = system("termux-media-player play /data/data/com.termux/files/home/bellaciao.wav 2>/dev/null");
    if (result == 0) return true;
    
    // Método 2: Usar termux-tts (text-to-speech)
    result = system("termux-tts-speak 'Alarma' 2>/dev/null");
    if (result == 0) return true;
    
    // Método 3: Usar termux-notification con sonido
    result = system("termux-notification --sound --title 'Alarma' --content '¡Despertador!' 2>/dev/null");
    if (result == 0) return true;
    
    // Método 4: Beep del sistema
    result = system("tput bel 2>/dev/null");
    if (result == 0) return true;
    
    return false;
}

void AudioPlayer::triggerVibration() {
    // Vibración por 500ms
    system("termux-vibrate -d 500 2>/dev/null");
}