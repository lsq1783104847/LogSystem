#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <vector>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include "level.hpp"
#include "format_message.hpp"
#include "sink.hpp"
#include "buffer.hpp"
#include "asyn_worker.hpp"

#define MAX_MSG 4096

namespace log_system
{
    // 日志器模块，作用：组合之前已经实现的多个小模块的功能，最终形成供用户调用的接口
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name, const std::vector<LogSink::ptr> &sinks,
               Level::value val = Level::value::DEBUG, const std::string &fmt_str = "[%L][%D_%T][%f:%l][%N:%i][%m]%n")
            : _logger_name(logger_name), _fmt_str(fmt_str),
              _sinks(sinks.begin(), sinks.end()), _limit_out_level(val) {}
        // 供用户调用输出日志信息,成功输出返回0，未达到输出等级返回1，出错返回-1
        int log(Level::value val, const std::string &filename, size_t line, const char *msg, ...)
        {
            if (val < _limit_out_level)
                return 1;
            va_list p;
            va_start(p, msg);
            char msg_buffer[MAX_MSG] = {0};
            if (vsnprintf(msg_buffer, MAX_MSG - 1, msg, p) < 0)
                return -1;
            va_end(p);

            LogMsg log_msg(filename, line, time(nullptr), std::this_thread::get_id(), _logger_name, msg_buffer, val);
            std::string log_str = LogFmt(_fmt_str, log_msg).get_result();
            if (!log_mode(log_str))
                return -1;

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
        SynLogger(const std::string &logger_name, const std::vector<LogSink::ptr> &sinks,
                  Level::value val = Level::value::DEBUG, const std::string &fmt_str = "[%L][%D_%T][%f:%l][%N:%i][%m]%n")
            : Logger(logger_name, sinks, val, fmt_str) {}

    protected:
        bool log_mode(const std::string &log_str) override
        {
            if (log_str == "")
                return false;
            bool ret = true;
            for (auto &sink : _sinks)
                ret &= sink->log(log_str);
            return ret;
        }
    };

    // 异步日志器
    class AsynLogger : public Logger
    {
    public:
        using ptr = std::shared_ptr<AsynLogger>;
        AsynLogger(const std::string &logger_name, const std::vector<LogSink::ptr> &sinks,
                   Level::value val = Level::value::DEBUG, const std::string &fmt_str = "[%L][%D_%T][%f:%l][%N:%i][%m]%n")
            : Logger(logger_name, sinks, val, fmt_str) {}

    protected:
        bool log_mode(const std::string &log_str) override
        {
            if (log_str == "")
                return false;
            bool ret = true;
            for (auto &sink : _sinks)
            {
                ret &= AsynWorkerPool::get_instance(handle_buffer_data, thread_size)
                           .push(Buffer_data(sink, log_str));
            }
            return ret;
        }
        static const size_t thread_size = DEFAULT_ASYN_THREAD_SIZE;
        static bool handle_buffer_data(const Buffer_data &data)
        {
            return data._sink->log(data._log_str);
        }
    };

    // 使用建造者模式来构造Logger，让用户直接通过建造者来创建Logger，从而简化用户的使用
}

#endif