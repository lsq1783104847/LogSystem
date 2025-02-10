#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <vector>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <unordered_map>
#include "level.hpp"
#include "format_message.hpp"
#include "sink.hpp"
#include "buffer.hpp"
#include "asyn_worker.hpp"

#define MAX_MSG 4096 // 日志主体消息的最大大小

namespace log_system
{
#define DEFAULT_FMT_STR "[%L][%D_%T][%f:%l][%N:%i][%m]%n" // 默认的日志输出格式字符串
    // 日志器种类标识
    enum LoggerType
    {
        SYNC_LOGGER = 0,
        ASYNC_LOGGER
    };
    // 日志器模块，作用：组合其他模块的功能，最终供用户调用以实现日志的指定输出
    // 日志器基类，将来子类通过重写log_mode(const std::string &log_str)函数来实现不同的日志器类型
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const std::string &logger_name, const std::vector<LogSink::ptr> &sinks,
               Level::value val = Level::value::DEBUG, const std::string &fmt_str = DEFAULT_FMT_STR)
            : _logger_name(logger_name), _fmt_str(fmt_str),
              _sinks(sinks.begin(), sinks.end()), _limit_out_level(val) {}
        virtual ~Logger() {}
        // 供用户调用以输出日志信息,成功输出返回0，未达到输出等级返回1，出错返回-1
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

    protected:
        std::string _logger_name;         // 日志器名称
        std::vector<LogSink::ptr> _sinks; // 日志落地对象数组（支持日志同时向多个落地方向输出）
        Level::value _limit_out_level;    // 限制输出的日志等级
        std::string _fmt_str;             // 日志输出格式字符串
    };

    // 同步日志器
    class SynLogger : public Logger
    {
    public:
        friend class LoggerManager;
        using ptr = std::shared_ptr<SynLogger>;
        ~SynLogger() {}

    protected:
        SynLogger(const SynLogger &tp) = delete;
        SynLogger &operator=(const SynLogger &tp) = delete;
        SynLogger(const std::string &logger_name, const std::vector<LogSink::ptr> &sinks,
                  Level::value val = Level::value::DEBUG, const std::string &fmt_str = DEFAULT_FMT_STR)
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
        friend class LoggerManager;
        using ptr = std::shared_ptr<AsynLogger>;
        ~AsynLogger() {}

    protected:
        AsynLogger(const AsynLogger &tp) = delete;
        AsynLogger &operator=(const AsynLogger &tp) = delete;
        AsynLogger(const std::string &logger_name, const std::vector<LogSink::ptr> &sinks,
                   Level::value val = Level::value::DEBUG, const std::string &fmt_str = DEFAULT_FMT_STR)
            : Logger(logger_name, sinks, val, fmt_str) {}

    protected:
        bool log_mode(const std::string &log_str) override
        {
            if (log_str == "")
                return false;
            bool ret = true;
            for (auto &sink : _sinks)
                ret &= AsynWorkerPool::get_instance(handle_buffer_data, thread_size)
                           ->push(Buffer_data(sink, log_str));
            return ret;
        }

    protected:
        static const size_t thread_size; // 将来创建异步线程池时指定的异步工作线程数量
        // 将来传入异步工作线程池的日志数据处理的回调函数
        static bool handle_buffer_data(const Buffer_data &data)
        {
            return data._sink->log(data._log_str);
        }
    };
    const size_t AsynLogger::thread_size = DEFAULT_ASYN_THREAD_SIZE;

    // 日志器管理者，所有的日志器以名字作为唯一标识，全局内均有效，将来用户都通过LoggerManager来添加和获取Logger
    // LoggerManager设计为单例模式，将来全局内所有的Logger都通过LoggerManager来创建，用户不能自行创建Logger
    class LoggerManager
    {
    public:
        using ptr = std::shared_ptr<LoggerManager>;
        ~LoggerManager() {}
        // 根据传入的参数向LoggerManager添加新的logger，如果日志器已经存在或者logger_name为空或者发生其他错误则返回false，成功添加则返回true
        bool add_logger(const std::string &logger_name, LoggerType type = SYNC_LOGGER, const std::vector<LogSink::ptr> &sinks = {StdoutSink::get_sink()},
                        Level::value val = Level::value::DEBUG, const std::string &fmt_str = DEFAULT_FMT_STR)
        {
            if (logger_name == "")
                return false;
            std::unique_lock<std::mutex> loggers_lock(_loggers_mutex);
            if (_loggers_hash.find(logger_name) != _loggers_hash.end())
                return false;
            if (type == SYNC_LOGGER)
            {
                Logger::ptr tmp(new SynLogger(logger_name, sinks, val, fmt_str));
                _loggers_hash[logger_name] = tmp;
            }
            else if (type == ASYNC_LOGGER)
            {
                Logger::ptr tmp(new AsynLogger(logger_name, sinks, val, fmt_str));
                _loggers_hash[logger_name] = tmp;
            }
            return true;
        }
        // 根据logger_name获取已经存在的Logger,获取失败返回nullptr，成功则返回指向该Logger的智能指针
        Logger::ptr get_logger(const std::string &logger_name)
        {
            if (logger_name == "")
                return Logger::ptr(nullptr);
            std::unique_lock<std::mutex> loggers_lock(_loggers_mutex);
            if (_loggers_hash.find(logger_name) != _loggers_hash.end())
                return _loggers_hash[logger_name];
            return Logger::ptr(nullptr);
        }
        // 用户通过get_instance()来获取唯一的单例对象使用
        static LoggerManager::ptr get_instance()
        {
            static LoggerManager::ptr logger_manager(new LoggerManager());
            return logger_manager;
        }

    private:
        LoggerManager(const LoggerManager &tp) = delete;
        LoggerManager &operator=(const LoggerManager &tp) = delete;
        LoggerManager() { add_logger("root"); } // LoggerManager单例创建时就自带一个名为"root"的同步日志器，其内部成员的值都是构建时传入的缺省值

    private:
        std::unordered_map<std::string, Logger::ptr> _loggers_hash; // 管理全局范围内的Logger，以logger_name为唯一标识
        std::mutex _loggers_mutex;                                  // 互斥锁，保护对_loggers_hash操作的线程安全
    };
}

#endif