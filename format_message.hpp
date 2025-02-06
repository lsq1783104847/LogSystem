#ifndef FORMATER_HPP
#define FORMATER_HPP

#include <sstream>
#include <memory>
#include <thread>
#include "level.hpp"

namespace log_system
{
    // 日志消息类。包含一条日志中所需的所有内容
    struct LogMsg
    {
        using ptr = std::shared_ptr<LogMsg>;
        LogMsg() = default;
        LogMsg(const std::string &filename, size_t line, time_t time,
               std::thread::id tid, const std::string &logggername,
               const std::string &main_message, Level::value level)
            : _filename(filename), _line(line), _time(time), _tid(tid),
              _logggername(logggername), _main_message(main_message), _level(level) {}
        ~LogMsg() {}
        std::string _filename;     // 文件名
        size_t _line;              // 行号
        time_t _time;              // 时间戳
        std::thread::id _tid;      // 线程ID
        std::string _logggername;  // 日志器名称
        std::string _main_message; // 日志主体消息
        Level::value _level;       // 日志等级
    };

    // 格式化类，构造时传入将来输出日志时的格式化的字符串样式\
    以及一个包含日志所有信息的LogMsg对象\
    格式化的结果放在_result中,成功则为日志输出的结果字符串，失败则为空串
    /*
           %D 日期(年月日)
           %T 时间(时分秒)
           %t 缩进
           %i 线程id
           %L 日志级别
           %N 日志器名称
           %f 文件名
           %l 行号
           %m 日志消息
           %n 换行
           %% 表示一个'%'字符
    */
    class LogFmt
    {
    public:
        LogFmt(const std::string format_str, const LogMsg &msg)
        {
            int pos = 0;
            while (pos < format_str.size())
            {
                if (format_str[pos] == '%')
                {
                    if (pos + 1 >= format_str.size())
                    {
                        _result = "";
                        return;
                    }
                    switch (format_str[pos + 1])
                    {
                    case 'D':
                        _result += data_to_string(msg);
                        break;
                    case 'T':
                        _result += time_to_string(msg);
                        break;
                    case 't':
                        _result += '\t';
                        break;
                    case 'i':
                        _result += tid_to_string(msg);
                        break;
                    case 'L':
                        _result += level_to_string(msg);
                        break;
                    case 'N':
                        _result += logger_name_to_string(msg);
                        break;
                    case 'f':
                        _result += filename_to_string(msg);
                        break;
                    case 'l':
                        _result += line_to_string(msg);
                        break;
                    case 'm':
                        _result += message_to_string(msg);
                        break;
                    case 'n':
                        _result += '\n';
                        break;
                    case '%':
                        _result += '%';
                        break;
                    default:
                        _result = "";
                        return;
                    }
                    pos += 2;
                }
                else
                    _result += format_str[pos++];
            }
        }
        std::string get_result()
        {
            return _result;
        }

    private:
        std::string data_to_string(const LogMsg &msg)
        {
            struct tm data;
            localtime_r(&msg._time, &data);
            std::stringstream sstr;
            sstr << data.tm_year + 1900 << '-';
            sstr << data.tm_mon + 1 << '-';
            sstr << data.tm_mday;
            return sstr.str();
        }
        std::string time_to_string(const LogMsg &msg)
        {
            struct tm time;
            localtime_r(&msg._time, &time);
            std::stringstream sstr;
            sstr << time.tm_hour << ':';
            sstr << time.tm_min << ':';
            sstr << time.tm_sec;
            return sstr.str();
        }
        std::string tid_to_string(const LogMsg &msg)
        {
            std::stringstream sstr;
            sstr << msg._tid;
            return sstr.str();
        }
        std::string level_to_string(const LogMsg &msg)
        {
            return Level::to_string(msg._level);
        }
        std::string logger_name_to_string(const LogMsg &msg)
        {
            return msg._logggername;
        }
        std::string filename_to_string(const LogMsg &msg)
        {
            return msg._filename;
        }
        std::string line_to_string(const LogMsg &msg)
        {
            return std::to_string(msg._line);
        }
        std::string message_to_string(const LogMsg &msg)
        {
            return msg._main_message;
        }

    private:
        std::string _result;
    };
}

#endif