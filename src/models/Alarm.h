#ifndef ALARM_H
#define ALARM_H

#include <string>

struct Alarm {
    std::string id;
    int hour;
    int minute;
    std::string label;
    bool enabled;
    bool vibrate;
    std::string sound_file;
    bool triggered_today = false;
};

#endif