#include "TimeUtils.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>

CurrentTime TimeUtils::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    std::tm* local_tm = std::localtime(&now_t);
    
    CurrentTime time;
    time.hour = local_tm->tm_hour;
    time.minute = local_tm->tm_min;
    time.second = local_tm->tm_sec;
    
    return time;
}

std::string TimeUtils::generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    const char* hex = "0123456789abcdef";
    std::string uuid = "alarm_";
    
    for (int i = 0; i < 8; i++) {
        uuid += hex[dis(gen)];
    }
    
    return uuid;
}

std::string TimeUtils::formatTime(int hour, int minute) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hour << ":"
        << std::setfill('0') << std::setw(2) << minute;
    return oss.str();
}