#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <string>
#include "Timetool.hpp"

class UTILS
{
public:
    static void printLog(int size)
    {
        //
    }

    static std::string getCurrentDir()
    {  
       /*
        char buff[256];
        GetModuleFileNameA(NULL, buff, 256);
        std::string::size_type position = std::string(buff).find_last_of("\\/");
        return std::string(buff).substr(0, position);
*/
       return ".";
    }
};

#endif // UTILS_H
