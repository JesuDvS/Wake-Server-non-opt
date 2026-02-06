#include "AlarmManager.h"
#include "../utils/TimeUtils.h"
#include "../utils/Logger.h"
#include "../models/AlarmStorage.cpp"
#include <chrono>
#include <algorithm>

AlarmManager::AlarmManager() 
    : audio_player_(std::make_unique<AudioPlayer>()) {
    loadAlarms();
}

AlarmManager::~AlarmManager() {
    stop();
}

void AlarmManager::start() {
    running_ = true;
    check_thread_ = std::thread(&AlarmManager::checkAlarmsLoop, this);
    Logger::info("AlarmManager iniciado");
}

void AlarmManager::stop() {
    running_ = false;
    if (check_thread_.joinable()) {
        check_thread_.join();
    }
    Logger::info("AlarmManager detenido");
}

std::string AlarmManager::createAlarm(int hour, int minute, const std::string& label,
                                     bool vibrate, const std::string& sound_file) {
    std::lock_guard<std::mutex> lock(alarms_mutex_);
    
    Alarm alarm;
    alarm.id = TimeUtils::generateUUID();
    alarm.hour = hour;
    alarm.minute = minute;
    alarm.label = label;
    alarm.enabled = true;
    alarm.vibrate = vibrate;
    alarm.sound_file = sound_file;
    
    alarms_.push_back(alarm);
    saveAlarms();
    
    Logger::info("Alarma creada: " + alarm.id + " para " + 
                 std::to_string(hour) + ":" + std::to_string(minute));
    
    return alarm.id;
}

bool AlarmManager::deleteAlarm(const std::string& id) {
    std::lock_guard<std::mutex> lock(alarms_mutex_);
    
    auto it = std::find_if(alarms_.begin(), alarms_.end(),
                          [&id](const Alarm& a) { return a.id == id; });
    
    if (it != alarms_.end()) {
        alarms_.erase(it);
        saveAlarms();
        Logger::info("Alarma eliminada: " + id);
        return true;
    }
    
    return false;
}

bool AlarmManager::toggleAlarm(const std::string& id) {
    std::lock_guard<std::mutex> lock(alarms_mutex_);
    
    auto it = std::find_if(alarms_.begin(), alarms_.end(),
                          [&id](const Alarm& a) { return a.id == id; });
    
    if (it != alarms_.end()) {
        it->enabled = !it->enabled;
        saveAlarms();
        Logger::info("Alarma " + id + " " + (it->enabled ? "activada" : "desactivada"));
        return true;
    }
    
    return false;
}

nlohmann::json AlarmManager::getAllAlarms() {
    std::lock_guard<std::mutex> lock(alarms_mutex_);
    
    nlohmann::json result = nlohmann::json::array();
    for (const auto& alarm : alarms_) {
        nlohmann::json j;
        j["id"] = alarm.id;
        j["hour"] = alarm.hour;
        j["minute"] = alarm.minute;
        j["label"] = alarm.label;
        j["enabled"] = alarm.enabled;
        j["vibrate"] = alarm.vibrate;
        j["sound_file"] = alarm.sound_file;
        j["ringing"] = (current_ringing_alarm_ == alarm.id);
        result.push_back(j);
    }
    
    return result;
}

void AlarmManager::stopCurrentAlarm() {
    if (alarm_ringing_) {
        audio_player_->stop();
        alarm_ringing_ = false;
        current_ringing_alarm_.clear();
        Logger::info("Alarma detenida por el usuario");
    }
}

bool AlarmManager::isAlarmRinging() const {
    return alarm_ringing_;
}

std::string AlarmManager::getCurrentRingingAlarmLabel() {
    std::lock_guard<std::mutex> lock(alarms_mutex_);
    
    if (!alarm_ringing_ || current_ringing_alarm_.empty()) {
        return "";
    }
    
    auto it = std::find_if(alarms_.begin(), alarms_.end(),
                          [this](const Alarm& a) { return a.id == current_ringing_alarm_; });
    
    if (it != alarms_.end()) {
        return it->label;
    }
    
    return "Alarma";
}

void AlarmManager::checkAlarmsLoop() {
    while (running_) {
        auto now = TimeUtils::getCurrentTime();
        
        {
            std::lock_guard<std::mutex> lock(alarms_mutex_);
            
            for (auto& alarm : alarms_) {
                if (!alarm.enabled) continue;
                
                if (now.hour == alarm.hour && now.minute == alarm.minute && 
                    !alarm.triggered_today) {
                    
                    triggerAlarm(alarm);
                    alarm.triggered_today = true;
                }
                
                // Reset diario a medianoche
                if (now.hour == 0 && now.minute == 0) {
                    alarm.triggered_today = false;
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }
}

void AlarmManager::triggerAlarm(const Alarm& alarm) {
    Logger::info("🔔 ALARMA ACTIVADA EN SERVIDOR: " + alarm.label);
    
    alarm_ringing_ = true;
    current_ringing_alarm_ = alarm.id;
    
    // Reproducir sonido EN EL SERVIDOR (Termux)
    audio_player_->play(alarm.sound_file, alarm.vibrate);
    
    // Auto-detener después de 5 minutos
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::minutes(5));
        if (alarm_ringing_) {
            Logger::warning("Alarma auto-detenida después de 5 minutos");
            stopCurrentAlarm();
        }
    }).detach();
}

void AlarmManager::saveAlarms() {
    AlarmStorage::save(alarms_);
}

void AlarmManager::loadAlarms() {
    alarms_ = AlarmStorage::load();
    Logger::info("Cargadas " + std::to_string(alarms_.size()) + " alarmas");
}