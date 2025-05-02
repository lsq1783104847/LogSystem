#ifndef LOG_SYSTEM_BUFFER_HPP
#define LOG_SYSTEM_BUFFER_HPP

#include <vector>
#include <string>
#include "sink.hpp"

namespace log_system
{
#define BUFFER_SIZE 1024 // 缓冲区的最大大小

    // Buffer_data为Buffer缓冲区中的元素,其包含了一个要输出的日志数据字符串和一个日志落地对象，将来就通过该落地对象将日志输出
    struct Buffer_data
    {
        Buffer_data() : _log_str(""), _sink(nullptr) {}
        Buffer_data(const LogSink::ptr &sink, const std::string &log_str) : _sink(sink), _log_str(log_str) {}
        std::string _log_str;
        LogSink::ptr _sink;
    };

    // 缓冲区类，为异步工作线程池的双缓冲区实现提供了各种调用接口，但其本身并不保证线程安全
    class Buffer
    {
    public:
        Buffer() : _reader_idx(0), _writer_idx(0), _buffer(BUFFER_SIZE) {}
        ~Buffer() {}
        bool is_full() { return _writer_idx >= BUFFER_SIZE; }  // 判断缓冲区是否为满
        bool is_empty() { return _reader_idx == _writer_idx; } // 判断缓冲区是否为空
        void reset() { _reader_idx = _writer_idx = 0; }        // 将缓冲区的读写指针置0
        // 用于交换两个缓冲区，交换读写指针和缓冲区内的数据
        void swap(Buffer &buffer)
        {
            std::swap(_reader_idx, buffer._reader_idx);
            std::swap(_writer_idx, buffer._writer_idx);
            _buffer.swap(buffer._buffer);
        }
        // 向缓冲区中插入数据，成功返回true，失败返回false
        bool push(const Buffer_data &buffer_data)
        {
            if (is_full())
                return false;
            _buffer[_writer_idx++] = buffer_data;
            return true;
        }
        // 从缓冲区中读取数据，失败返回空的Buffer_data
        Buffer_data pop()
        {
            if (is_empty())
                return Buffer_data();
            return _buffer[_reader_idx++];
        }

    private:
        std::vector<Buffer_data> _buffer; // 存放缓冲区数据的数组
        size_t _reader_idx;               // 标识当前的读位置
        size_t _writer_idx;               // 标识当前的写位置
    };
}

#endif