#ifndef WAKELOCK_MANAGER_H
#define WAKELOCK_MANAGER_H

#include <string>
#include <chrono>

class WakeLockManager {
public:
    WakeLockManager();
    ~WakeLockManager();
    
    bool acquire();
    bool release();
    void refresh();
    
private:
    bool executeTermuxAPI(const std::string& command);
    void writeTouchFile();
    
    bool is_acquired_;
    std::chrono::steady_clock::time_point last_refresh_;
};

#endif