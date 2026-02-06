#include "Alarm.h"
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../utils/Logger.h"

using json = nlohmann::json;

class AlarmStorage {
public:
    static void save(const std::vector<Alarm>& alarms) {
        json j = json::array();
        
        for (const auto& alarm : alarms) {
            json item;
            item["id"] = alarm.id;
            item["hour"] = alarm.hour;
            item["minute"] = alarm.minute;
            item["label"] = alarm.label;
            item["enabled"] = alarm.enabled;
            item["vibrate"] = alarm.vibrate;
            item["sound_file"] = alarm.sound_file;
            j.push_back(item);
        }
        
        std::ofstream file("alarms.json");
        if (file.is_open()) {
            file << j.dump(2);
            file.close();
            Logger::info("Alarmas guardadas en alarms.json");
        }
    }
    
    static std::vector<Alarm> load() {
        std::vector<Alarm> alarms;
        
        std::ifstream file("alarms.json");
        if (!file.is_open()) {
            Logger::info("No se encontró alarms.json, iniciando vacío");
            return alarms;
        }
        
        try {
            json j;
            file >> j;
            
            for (const auto& item : j) {
                Alarm alarm;
                alarm.id = item["id"];
                alarm.hour = item["hour"];
                alarm.minute = item["minute"];
                alarm.label = item.value("label", "Alarma");
                alarm.enabled = item.value("enabled", true);
                alarm.vibrate = item.value("vibrate", true);
                alarm.sound_file = item.value("sound_file", "default");
                alarm.triggered_today = false;
                
                alarms.push_back(alarm);
            }
            
        } catch (const std::exception& e) {
            Logger::error("Error cargando alarmas: " + std::string(e.what()));
        }
        
        file.close();
        return alarms;
    }
};