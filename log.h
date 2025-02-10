#ifndef LOG_H
#define LOG_H

#include "logger.hpp"

namespace log_system
{
    template <typename SinkType, typename... Args>
    LogSink::ptr get_sink(Args &&...args) { return SinkType::get_sink(std::forward<Args>(args)...); }

    Logger::ptr get_logger(const std::string &name) { return LoggerManager::get_instance()->get_logger(name); }

    bool add_logger(const std::string &logger_name, LoggerType type = SYNC_LOGGER, const std::vector<LogSink::ptr> &sinks = {StdoutSink::get_sink()},
                    Level::value val = Level::value::DEBUG, const std::string &fmt_str = DEFAULT_FMT_STR)
    {
        return LoggerManager::get_instance()->add_logger(logger_name, type, sinks, val, fmt_str);
    }

#define LOG(logger, level, msg, ...) (logger)->log(level, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define LOG_DEBUG(logger, msg, ...) LOG(logger, log_system::Level::value::DEBUG, msg, ##__VA_ARGS__)
#define LOG_INFO(logger, msg, ...) LOG(logger, log_system::Level::value::INFO, msg, ##__VA_ARGS__)
#define LOG_WARN(logger, msg, ...) LOG(logger, log_system::Level::value::WARN, msg, ##__VA_ARGS__)
#define LOG_ERROR(logger, msg, ...) LOG(logger, log_system::Level::value::ERROR, msg, ##__VA_ARGS__)
#define LOG_FATAL(logger, msg, ...) LOG(logger, log_system::Level::value::FATAL, msg, ##__VA_ARGS__)
}

#endif