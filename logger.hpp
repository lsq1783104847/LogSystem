#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <vector>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include "level.hpp"
#include "sink.hpp"

#define MAX_MSG 4096

namespace log_system
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name, const std::vector<LogSink> &sinks, Level::value val = Level::value::DEBUG)
            : _limit_out_level(val), _logger_name(logger_name), _sinks(sinks.begin(), sinks.end())
        {
        }
        // 供用户调用输出日志信息
        int log(Level::value val, const std::string &filename, size_t line, const char *msg, ...)
        {
            if (val < _limit_out_level)
                return true;
            va_list p;
            va_start(p, msg);
            char buffer[MAX_MSG];
            if (vsnprintf(buffer, MAX_MSG, msg, p) < 0)
                return false;
            va_end(p);
            // TODO
            return true;
        }

    protected:
        virtual bool log_mode(const std::string &msg) = 0;
        std::mutex _mutex;
        std::string _logger_name;
        std::vector<LogSink> _sinks;
        Level::value _limit_out_level;
    };

    class SynLogger : public Logger
    {
    protected:
        bool log_mode(const std::string &msg) override
        {
            for (auto &sink : _sinks)
                sink.log(msg);
        }
    };
}

#endif