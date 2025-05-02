#ifndef LOG_SYSTEM_LOG_H
#define LOG_SYSTEM_LOG_H

#include "logger.hpp"

namespace log_system
{
    // 该文件内主要是对已经实现的功能进行的封装，目的是为用户提供更简便的操作

    // 根据类型和传入参数获取日志落地对象，失败则返回nullptr
    template <typename SinkType, typename... Args>
    LogSink::ptr get_sink(Args &&...args) { return SinkType::get_sink(std::forward<Args>(args)...); }
    // 根据日志器名称直接获取日志器，失败则返回nullptr
    Logger::ptr get_logger(const std::string &name) { return LoggerManager::get_instance()->get_logger(name); }
    // 根据传入的参数创建新的日志器，若日志器已经存在或者发生错误则返回false
    bool add_logger(const std::string &logger_name, LoggerType type = SYNC_LOGGER, const std::vector<LogSink::ptr> &sinks = {StdoutSink::get_sink()},
                    Level::value val = Level::value::DEBUG, const std::string &fmt_str = DEFAULT_FMT_STR)
    {
        return LoggerManager::get_instance()->add_logger(logger_name, type, sinks, val, fmt_str);
    }
// 传入日志器和日志等级以及要输出的日志主体信息的格式化字符串和参数，用传入的日志器进行日志的落地输出
#define LOG(logger, level, msg, ...) (logger)->log(level, __FILE__, __LINE__, msg, ##__VA_ARGS__)
// 以下接口是封装的上一接口，是省略传入日志输出等级的实现
#define LOG_DEBUG(logger, msg, ...) LOG(logger, log_system::Level::value::DEBUG, msg, ##__VA_ARGS__)
#define LOG_INFO(logger, msg, ...) LOG(logger, log_system::Level::value::INFO, msg, ##__VA_ARGS__)
#define LOG_WARN(logger, msg, ...) LOG(logger, log_system::Level::value::WARN, msg, ##__VA_ARGS__)
#define LOG_ERROR(logger, msg, ...) LOG(logger, log_system::Level::value::ERROR, msg, ##__VA_ARGS__)
#define LOG_FATAL(logger, msg, ...) LOG(logger, log_system::Level::value::FATAL, msg, ##__VA_ARGS__)
}

#endif