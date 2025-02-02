#ifndef SINK_HPP
#define SINK_HPP

#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <memory>
#include "util.hpp"

namespace log_system
{
    // 文件落地基类
    // 通过重写log()函数实现不同的日志落地方向
    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;
        virtual bool log(const std::string &msg) = 0;
        virtual ~LogSink() = 0;
    };
    // 标准输出落地类
    // 将日志输出到标准输出中
    class StdoutSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<StdoutSink>;
        bool log(const std::string &msg) override
        {
            if (!std::cout.good())
                return false;
            std::cout << msg;
            return std::cout.good();
        }
    };
    // 指定文件落地类
    // 将日志输出到指定的文件中
    class FileSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<FileSink>;
        FileSink(const std::string &path)
        {
            // 判断文件所在路径是否存在，不存在就先创建
            _state = Util::create_dir(Util::file_dir(path));
            if (_state)
            {
                _ofs.open(path, std::ios::binary | std::ios::app);
                _state = _ofs.is_open();
            }
        }
        bool log(const std::string &msg) override
        {
            if (!_state || !_ofs.good())
                return false;
            _ofs << msg;
            return _ofs.good();
        }

    private:
        std::ofstream _ofs;
        bool _state;
    };
    // 滚动文件落地类
     // 根据传入的基础文件名和最大大小限制，先将基础文件名结合当前时间（以秒为单位）形成完整的文件名，再将日志输出到该文件中\
    当前时间与日志目标输出文件的创建时间在同一秒内，则日志文件不发生滚动，不在同一秒内且日志文件超过最大大小限制才触发滚动\
    滚动后又以当前时间结合基础文件名创建新的文件，再将日志输出到新的文件中
    class RollFileSinkBySize : public LogSink
    {
    public:
        using ptr = std::shared_ptr<RollFileSinkBySize>;
        RollFileSinkBySize(const std::string &path, long long max_size) : _max_size(max_size)
        {
            // 判断文件所在路径是否存在，不存在就先创建
            _fdir_path = Util::file_dir(path);
            _state = Util::create_dir(_fdir_path);
            if (_state)
            {
                _base_fname = Util::get_fname(path);
                if (_base_fname == "")
                    _state = false;
            }
            if (_state)
            {
                _cur_filename = get_filename_by_time();
                _ofs.open(_cur_filename, std::ios::binary | std::ios::app);
                _state = _ofs.is_open();
            }
        }
        bool log(const std::string &msg) override
        {
            if (!_state || !_ofs.good())
                return false;
            _ofs << msg;
            long long fsize = get_file_size();
            if (fsize == -1)
                return false;
            else if (fsize >= _max_size && _last_time != time(nullptr))
            {
                _cur_filename = get_filename_by_time();
                _ofs.close();
                _ofs.open(_cur_filename, std::ios::binary | std::ios::app);
                _state = _ofs.is_open();
            }
            return _ofs.good();
        }

    private:
        // 根据当前时间动态获取文件名,并修改_last_time
        std::string get_filename_by_time()
        {
            _last_time = time(nullptr);
            struct tm now;
            localtime_r(&_last_time, &now);
            std::stringstream sstr;
            sstr << _fdir_path;
            sstr << now.tm_year + 1900 << '-';
            sstr << now.tm_mon + 1 << '-';
            sstr << now.tm_mday << '_';
            sstr << now.tm_hour << ':';
            sstr << now.tm_min << ':';
            sstr << now.tm_sec << '_';
            sstr << _base_fname;
            return sstr.str();
        }
        // 获取文件的大小
        long long get_file_size()
        {
            struct stat att;
            if (stat(_cur_filename.c_str(), &att) == -1)
                return -1;
            else
                return att.st_size;
        }

        std::ofstream _ofs;
        std::string _base_fname;
        std::string _fdir_path;
        std::string _cur_filename;
        time_t _last_time;   // 记录_cur_filename创建时的时间戳
        long long _max_size; // 以字节为单位
        bool _state;
    };

    // ...支持在此处扩展，可以根据使用需求实现更多的落地方向子类使得日志可以向更多的位置输出

    // 统一使用工厂来创建落地类的对象（简单工厂模式的应用）
    class SinkFactory
    {
    public:
        template <typename SinkType, typename... Args>
        static LogSink::ptr create(Args &&...args)
        {
            return std::make_shared<SinkType>(std::forward<Args>(args)...);
        }
    };
}

#endif