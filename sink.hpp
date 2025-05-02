#ifndef LOG_SYSTEM_SINK_HPP
#define LOG_SYSTEM_SINK_HPP

#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <mutex>
#include <memory>
#include <unordered_map>
#include "util.hpp"

namespace log_system
{
    // 日志落地模块
    // 日志落地基类,将来子类通过重写log()函数实现不同的日志落地方向
    // 所有日志落地的对象内部自行保证多线程使用同一对象进行落地时的线程安全,即保证了多线程使用同一对象在调用log()函数进行日志落地时的线程安全
    // 日志落地对象根据日志落地的位置保证全局唯一性,所有的日志落地对象都不可以直接创建使用，必须由每个类的静态成员函数get_sink()进行创建和获取，如果get_sink()返回nullptr就表示获取失败
    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;
        virtual bool log(const std::string &msg) = 0;
        virtual ~LogSink() {};
    };
    // 标准输出落地类，将日志输出到标准输出中
    class StdoutSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<StdoutSink>;
        ~StdoutSink() {}
        bool log(const std::string &msg) override
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (!std::cout.good())
                return false;
            std::cout << msg << std::flush;
            return std::cout.good();
        }
        static StdoutSink::ptr get_sink()
        {
            static StdoutSink::ptr sink(new StdoutSink());
            return sink;
        }

    private:
        StdoutSink() {}
        StdoutSink(const StdoutSink &tp) = delete;
        StdoutSink &operator=(const StdoutSink &tp) = delete;

    private:
        std::mutex _mutex; // 互斥锁，用于保证同一对象多线程下调用log()函数时的线程安全
    };

    // 指定文件落地类，将日志输出到指定的文件中
    class FileSink : public LogSink
    {
    public:
        using ptr = std::shared_ptr<FileSink>;
        ~FileSink()
        {
            if (_ofs.is_open())
                _ofs.close();
        }
        virtual bool log(const std::string &msg) override
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (!_state || !_ofs.good())
                return false;
            _ofs << msg << std::flush;
            return _ofs.good();
        }
        static FileSink::ptr get_sink(const std::string &path)
        {
            std::string absolute_path = Util::path_transform(path);
            if (absolute_path == "")
                return FileSink::ptr(nullptr);
            std::unique_lock<std::mutex> filehash_lock(_filehash_mutex);
            if (_filehash.find(absolute_path) != _filehash.end())
                return _filehash[absolute_path];
            FileSink::ptr tmp(new FileSink(absolute_path));
            if (tmp->_state == false)
                return FileSink::ptr(nullptr);
            _filehash[absolute_path] = tmp;
            return _filehash[absolute_path];
        }

    protected:
        FileSink() = default;
        FileSink(const FileSink &tp) = delete;
        FileSink &operator=(const FileSink &tp) = delete;
        FileSink(const std::string &path)
        {
            std::string absolute_path = Util::path_transform(path);
            if (absolute_path == "")
                _state = false;
            // 判断文件所在路径是否存在，不存在就先创建
            if (_state)
                _state = Util::create_dir(Util::file_dir(absolute_path));
            if (_state)
            {
                _ofs.open(absolute_path, std::ios::binary | std::ios::app);
                _state = _ofs.is_open();
            }
        }

    protected:
        std::mutex _mutex;  // 互斥锁，用于保证同一对象多线程下调用log()函数时的线程安全
        std::ofstream _ofs; // 文件流对象，用于管理打开的文件
        bool _state = true; // 状态标志位，标识当前对象的状态

    private:
        static std::unordered_map<std::string, FileSink::ptr> _filehash; // 全局范围内的所有FileSink对象交给_filehash统一管理，以保证FileSink对象全局范围内的唯一性
        static std::mutex _filehash_mutex;                               // 保证多线程操作_filehash时的线程安全
    };
    std::unordered_map<std::string, FileSink::ptr> FileSink::_filehash;
    std::mutex FileSink::_filehash_mutex;

    // 滚动文件落地类 \
    根据传入的基础文件名和最大大小限制，先将基础文件名结合当前时间（以秒为单位）形成完整的文件名，再将日志输出到该文件中\
    当前时间与日志目标输出文件的创建时间在同一秒内，则日志文件不发生滚动，不在同一秒内且日志文件超过最大大小限制才触发滚动\
    滚动后又以当前时间结合基础文件名创建新的文件，再将日志输出到新的文件中
    class RollFileSinkBySize : public FileSink
    {
#define DEFAULT_MAX_SIZE (1024 * 1024) // 滚动文件默认的最大大小
    public:
        using ptr = std::shared_ptr<RollFileSinkBySize>;
        ~RollFileSinkBySize() = default;
        bool log(const std::string &msg) override
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (!_state || !_ofs.good())
                return false;
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
            if (!_state || !_ofs.good())
                return false;
            _ofs << msg << std::flush;
            return _ofs.good();
        }
        static RollFileSinkBySize::ptr get_sink(const std::string &path, long long max_size = DEFAULT_MAX_SIZE)
        {
            std::string absolute_path = Util::path_transform(path);
            if (absolute_path == "")
                return RollFileSinkBySize::ptr(nullptr);
            std::unique_lock<std::mutex> roll_filehash_lock(_roll_filehash_mutex);
            if (_roll_filehash.find(absolute_path) != _roll_filehash.end())
                return _roll_filehash[absolute_path];
            RollFileSinkBySize::ptr tmp(new RollFileSinkBySize(absolute_path, max_size));
            if (tmp->_state == false)
                return RollFileSinkBySize::ptr(nullptr);
            _roll_filehash[absolute_path] = tmp;
            return _roll_filehash[absolute_path];
        }

    private:
        RollFileSinkBySize(const RollFileSinkBySize &tp) = delete;
        RollFileSinkBySize &operator=(const RollFileSinkBySize &tp) = delete;
        RollFileSinkBySize(const std::string &path, long long max_size = DEFAULT_MAX_SIZE) : _max_size(max_size)
        {
            std::string absolute_path = Util::path_transform(path);
            if (absolute_path == "")
                _state = false;
            // 判断文件所在路径是否存在，不存在就先创建
            if (_state)
            {
                _fdir_path = Util::file_dir(absolute_path);
                if (_fdir_path[_fdir_path.size() - 1] != '/')
                    _fdir_path.push_back('/');
                _state = Util::create_dir(_fdir_path);
            }
            if (_state)
            {
                _base_fname = Util::get_fname(absolute_path);
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
        // 获取文件的大小,返回-1表示失败
        long long get_file_size()
        {
            struct stat att;
            if (stat(_cur_filename.c_str(), &att) == -1)
                return -1;
            else
                return att.st_size;
        }

    protected:
        std::string _base_fname;   // 文件名（仅仅只含文件的名字，不含路径）
        std::string _fdir_path;    // 文件所处的目录的路径
        std::string _cur_filename; // 当前文件流所管理的完整文件名（加上了时间构造出来的完整文件名路径）
        time_t _last_time;         // 记录_cur_filename创建时的时间戳
        long long _max_size;       // 滚动文件的最大大小(以字节为单位)

    private:
        static std::unordered_map<std::string, RollFileSinkBySize::ptr> _roll_filehash; // 全局范围内的所有RollFileSinkBySize对象交给_roll_filehash统一管理，以保证RollFileSinkBySize对象全局范围内的唯一性
        static std::mutex _roll_filehash_mutex;                                         // 保证多线程操作_roll_filehash时的线程安全
    };
    std::unordered_map<std::string, RollFileSinkBySize::ptr> RollFileSinkBySize::_roll_filehash;
    std::mutex RollFileSinkBySize::_roll_filehash_mutex;

    // ...支持在此处扩展，可以根据使用需求自行实现更多的落地方向子类使得日志可以向更多的位置输出
}

#endif