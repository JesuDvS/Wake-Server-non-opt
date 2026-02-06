#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <string>

struct CurrentTime {
    int hour;
    int minute;
    int second;
};

class TimeUtils {
public:
    static CurrentTime getCurrentTime();
    static std::string generateUUID();
    static std::string formatTime(int hour, int minute);
};

#endif