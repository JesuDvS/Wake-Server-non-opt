#include "WakeLockManager.h"
#include "../utils/Logger.h"
#include <cstdlib>
#include <fstream>
#include <unistd.h>

WakeLockManager::WakeLockManager() : is_acquired_(false) {}

WakeLockManager::~WakeLockManager() {
    release();
}

bool WakeLockManager::acquire() {
    if (is_acquired_) return true;
    
    // Método 1: Termux-wake-lock (requiere termux-api)
    bool success = executeTermuxAPI("termux-wake-lock");
    
    // Método 2: Crear archivo de actividad continua
    writeTouchFile();
    
    if (success) {
        is_acquired_ = true;
        last_refresh_ = std::chrono::steady_clock::now();
        Logger::info("WakeLock adquirido");
    } else {
        Logger::warning("No se pudo adquirir WakeLock (termux-api no disponible)");
    }
    
    return true; // Continuar aunque falle
}

bool WakeLockManager::release() {
    if (!is_acquired_) return true;
    
    executeTermuxAPI("termux-wake-unlock");
    is_acquired_ = false;
    
    Logger::info("WakeLock liberado");
    return true;
}

void WakeLockManager::refresh() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_refresh_).count();
    
    if (elapsed > 25) {
        writeTouchFile();
        last_refresh_ = now;
    }
}

bool WakeLockManager::executeTermuxAPI(const std::string& command) {
    int result = system(command.c_str());
    return (result == 0);
}

void WakeLockManager::writeTouchFile() {
    // Escribir archivo para simular actividad
    std::ofstream touch_file("/data/data/com.termux/files/home/.keep_alive");
    if (touch_file.is_open()) {
        touch_file << std::chrono::system_clock::now().time_since_epoch().count();
        touch_file.close();
    }
    
    // Generar salida stdout para evitar suspensión
    std::cout << "\r" << std::flush;
}