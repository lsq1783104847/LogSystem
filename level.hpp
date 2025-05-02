#ifndef LOG_SYSTEM_LEVEL_HPP
#define LOG_SYSTEM_LEVEL_HPP

#include <string>

namespace log_system
{
    class Level
    {
    public:
        // 日志等级
        enum value
        {
            DEBUG = 0,
            INFO,
            WARN,
            ERROR,
            FATAL,
            OFF
        };
        // 根据日志等级转化成对应字符串
        static const std::string to_string(Level::value val)
        {
            if (val == DEBUG)
                return "DEBUG";
            else if (val == INFO)
                return "INFO";
            else if (val == WARN)
                return "WARN";
            else if (val == ERROR)
                return "ERROR";
            else if (val == FATAL)
                return "FATAL";
            else
                return "";
        }
    };
}

#endif