
#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <ctime>
#include <string>
#include <chrono>
#include <sstream>
#include <iostream>

class Timetool
{
public:
    static std::string GetCurrentTime()
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time_t);

        char buffer[128];
        strftime(buffer, sizeof(buffer), "%F %T", now_tm);

        std::ostringstream ss;
        ss.fill('0');

        ss << buffer;
        

        return ss.str();
    }
};

#endif //TIMESTAMP_H