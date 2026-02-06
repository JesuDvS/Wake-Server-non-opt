#include "httplib.h"
#include <resources.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include "core/AlarmManager.h"
#include "core/WakeLockManager.h"
#include "utils/Logger.h"

using json = nlohmann::json;

std::atomic<bool> server_running{true};

void keepAliveThread() {
    WakeLockManager wakelock;
    wakelock.acquire();
    
    while (server_running) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        wakelock.refresh();
        Logger::info("Keep-alive tick");
    }
    
    wakelock.release();
}

int main() {
    Logger::info("🚀 Iniciando servidor de alarmas...");
    
    // Hilo para mantener despierto el sistema
    std::thread keepalive(keepAliveThread);
    keepalive.detach();
    
    AlarmManager alarmManager;
    httplib::Server svr;

    // Servir interfaz principal
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        auto* resource = Resources::getResource("/index.html");
        if (resource) {
            res.set_content(resource->content, resource->mime_type);
        } else {
            res.status = 404;
        }
    });

    // API: Listar alarmas
    svr.Get("/api/alarms", [&alarmManager](const httplib::Request&, httplib::Response& res) {
        json response = alarmManager.getAllAlarms();
        res.set_content(response.dump(), "application/json");
    });

    // API: Crear alarma
    svr.Post("/api/alarms", [&alarmManager](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string alarm_id = alarmManager.createAlarm(
                body["hour"], 
                body["minute"],
                body.value("label", "Alarma"),
                body.value("vibrate", true),
                body.value("sound_file", "default")
            );
            
            json response;
            response["success"] = true;
            response["id"] = alarm_id;
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            json error;
            error["success"] = false;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json");
        }
    });

    // API: Eliminar alarma
    svr.Delete("/api/alarms/:id", [&alarmManager](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.path_params.at("id");
        bool success = alarmManager.deleteAlarm(id);
        
        json response;
        response["success"] = success;
        res.set_content(response.dump(), "application/json");
    });

    // API: Activar/desactivar alarma
    svr.Put("/api/alarms/:id/toggle", [&alarmManager](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.path_params.at("id");
        bool success = alarmManager.toggleAlarm(id);
        
        json response;
        response["success"] = success;
        res.set_content(response.dump(), "application/json");
    });

    // API: Detener alarma sonando
    svr.Post("/api/alarms/stop", [&alarmManager](const httplib::Request&, httplib::Response& res) {
        alarmManager.stopCurrentAlarm();
        json response;
        response["success"] = true;
        res.set_content(response.dump(), "application/json");
    });

    // Servir recursos estáticos
    svr.Get(R"(/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string path = "/" + req.matches[1].str();
        auto* resource = Resources::getResource(path);
        if (resource) {
            res.set_content(resource->content, resource->mime_type);
        } else {
            res.status = 404;
        }
    });

    // Iniciar el gestor de alarmas
    alarmManager.start();

    Logger::info("🌐 Servidor escuchando en http://localhost:8082");
    Logger::info("📱 Abre en Termux: http://127.0.0.1:8082");
    
    svr.listen("0.0.0.0", 8082);

    server_running = false;
    alarmManager.stop();
    
    return 0;
}