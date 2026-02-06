#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <nlohmann/json.hpp>
#include "../models/Alarm.h"
#include "AudioPlayer.h"

class AlarmManager {
public:
    AlarmManager();
    ~AlarmManager();
    
    void start();
    void stop();
    
    std::string createAlarm(int hour, int minute, const std::string& label, 
                           bool vibrate, const std::string& sound_file);
    bool deleteAlarm(const std::string& id);
    bool toggleAlarm(const std::string& id);
    nlohmann::json getAllAlarms();
    void stopCurrentAlarm();
    
    // Nuevos métodos para que el cliente sepa si hay alarma sonando
    bool isAlarmRinging() const;
    std::string getCurrentRingingAlarmLabel();

private:
    void checkAlarmsLoop();
    void triggerAlarm(const Alarm& alarm);
    void saveAlarms();
    void loadAlarms();
    
    std::vector<Alarm> alarms_;
    std::thread check_thread_;
    std::atomic<bool> running_{false};
    std::mutex alarms_mutex_;
    
    std::unique_ptr<AudioPlayer> audio_player_;
    std::atomic<bool> alarm_ringing_{false};
    std::string current_ringing_alarm_;
};

#endif