#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>

class Logger {
public:
    static void info(const std::string& message) {
        log("INFO", message);
    }
    
    static void warning(const std::string& message) {
        log("WARN", message);
    }
    
    static void error(const std::string& message) {
        log("ERROR", message);
    }
    
private:
    static void log(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto now_t = std::chrono::system_clock::to_time_t(now);
        auto local_tm = std::localtime(&now_t);
        
        std::cout << "["
                  << std::setfill('0') << std::setw(2) << local_tm->tm_hour << ":"
                  << std::setfill('0') << std::setw(2) << local_tm->tm_min << ":"
                  << std::setfill('0') << std::setw(2) << local_tm->tm_sec
                  << "] [" << level << "] " << message << std::endl;
    }
};

#endif