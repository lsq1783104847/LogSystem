#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <vector>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include "level.hpp"
#include "format_message.hpp"
#include "sink.hpp"

#define MAX_MSG 4096

namespace log_system
{
    // 日志器模块，作用：组合之前已经实现的多个小模块的功能，最终形成供用户调用的接口
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name, const std::string &fmt_str,
               const std::vector<LogSink::ptr> &sinks, Level::value val = Level::value::DEBUG)
            : _logger_name(logger_name), _fmt_str(fmt_str),
              _sinks(sinks.begin(), sinks.end()), _limit_out_level(val) {}
        // 供用户调用输出日志信息,成功输出返回0，未达到输出等级返回1，出错返回-1
        int log(Level::value val, const std::string &filename, size_t line, const char *msg, ...)
        {
            if (val < _limit_out_level)
                return 1;
            va_list p;
            va_start(p, msg);
            char buffer[MAX_MSG];
            if (vsnprintf(buffer, MAX_MSG, msg, p) < 0)
                return -1;
            va_end(p);

            LogMsg log_msg(filename, line, time(nullptr), std::this_thread::get_id(), _logger_name, buffer, val);
            std::string log_str = LogFmt(_fmt_str, log_msg).get_result();

            log_mode(log_str);

            return 0;
        }

    protected:
        virtual bool log_mode(const std::string &log_str) = 0;

        std::string _logger_name;
        std::vector<LogSink::ptr> _sinks;
        Level::value _limit_out_level;
        std::string _fmt_str;
    };

    // 同步日志器
    class SynLogger : public Logger
    {
    public:
        using ptr = std::shared_ptr<SynLogger>;
        SynLogger(const std::string &logger_name, const std::string &fmt_str,
                  const std::vector<LogSink::ptr> &sinks, Level::value val = Level::value::DEBUG)
            : Logger(logger_name, fmt_str, sinks, val) {}

    protected:
        bool log_mode(const std::string &log_str) override
        {
            std::unique_lock<std::mutex> lock(_mutex);
            for (auto &sink : _sinks)
                if (sink->log(log_str) == false)
                    return false;
            return true;
        }

    private:
        std::mutex _mutex;
    };

    // 异步日志器
    class AsynLogger : public Logger
    {
    public:
        using ptr = std::shared_ptr<AsynLogger>;
        AsynLogger(const std::string &logger_name, const std::string &fmt_str,
                   const std::vector<LogSink::ptr> &sinks, Level::value val = Level::value::DEBUG)
            : Logger(logger_name, fmt_str, sinks, val) {}

    protected:
        bool log_mode(const std::string &log_str) override
        {
            return true;
        }
    };
}

#endif