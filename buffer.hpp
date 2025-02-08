#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vector>
#include <string>
#include "sink.hpp"

namespace log_system
{
#define BUFFER_SIZE 1024 // 缓冲区的最大大小

    struct Buffer_data
    {
        Buffer_data() = default;
        Buffer_data(const LogSink::ptr &sink, const std::string &log_str) : _sink(sink), _log_str(log_str) {}
        std::string _log_str;
        LogSink::ptr _sink;
    };

    class Buffer
    {
    public:
        Buffer() : _reader_idx(0), _writer_idx(0), _buffer(BUFFER_SIZE) {}
        ~Buffer() {}
        bool is_full() { return _writer_idx >= BUFFER_SIZE; }
        bool is_empty() { return _reader_idx == _writer_idx; }
        void reset() { _reader_idx = _writer_idx = 0; }
        void swap(Buffer &buffer)
        {
            std::swap(_reader_idx, buffer._reader_idx);
            std::swap(_writer_idx, buffer._writer_idx);
            _buffer.swap(buffer._buffer);
        }
        bool push(const Buffer_data &buffer_data)
        {
            if (is_full())
                return false;
            _buffer[_writer_idx++] = buffer_data;
            return true;
        }
        Buffer_data pop()
        {
            if (is_empty())
                return Buffer_data();
            return _buffer[_reader_idx++];
        }

    private:
        std::vector<Buffer_data> _buffer;
        size_t _reader_idx;
        size_t _writer_idx;
    };
}

#endif