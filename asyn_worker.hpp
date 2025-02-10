#ifndef ASYN_WORKER_HPP
#define ASYN_WORKER_HPP

#include <mutex>
#include <thread>
#include <memory>
#include <condition_variable>
#include <functional>
#include "buffer.hpp"

namespace log_system
{
#define DEFAULT_ASYN_THREAD_SIZE 2
    // 设计为单例，将来所有的Asynlogger共用同一套异步工作线程池
    class AsynWorkerPool
    {
    public:
        using func_t = std::function<bool(const Buffer_data &buffer)>;
        using ptr = std::shared_ptr<AsynWorkerPool>;
        ~AsynWorkerPool()
        {
            for (auto &thread : _threads)
                thread.join();
        }
        bool push(const Buffer_data &buffer_data)
        {
            std::unique_lock<std::mutex> push_lock(_push_mutex);
            _push_cond.wait(push_lock, [&]()
                            { return !_push_tasks.is_full(); });
            if (!_push_tasks.push(buffer_data))
                return false;
            _pop_cond.notify_all();
            return true;
        }
        static AsynWorkerPool::ptr get_instance(func_t func, size_t thread_size = DEFAULT_ASYN_THREAD_SIZE)
        {
            static AsynWorkerPool::ptr awp(new AsynWorkerPool(func, thread_size));
            return awp;
        }

    private:
        AsynWorkerPool(func_t func, size_t thread_size = DEFAULT_ASYN_THREAD_SIZE)
            : _func(func) // , _threads(thread_size, std::thread(&AsynWorkerPool::worker_thread, this))\
                            vector的这种方式使用方式并不适用于std::thread,因为vector是先通过给的值(第二个参数)构造一个对象, \
                            在开辟好空间后再通过先前构造好的对象，循环进行要创建的对象个数(第一个参数的值)次拷贝构造来填充vector开辟的空间中的值 \
                            而std::thread中,拷贝构造函数是被删除的函数,所以以上用法会报错,只能通过以下用法来插入一个个std::thread到vector
        {
            _threads.reserve(thread_size);
            for (size_t i = 0; i < thread_size; i++)
                _threads.push_back(std::thread(&AsynWorkerPool::worker_thread, this));
        }
        AsynWorkerPool(const AsynWorkerPool &tp) = delete;
        AsynWorkerPool &operator=(const AsynWorkerPool &tp) = delete;
        void worker_thread()
        {
            while (1)
            {
                Buffer_data data;
                {
                    std::unique_lock<std::mutex> pop_lock(_pop_mutex);
                    if (_pop_tasks.is_empty())
                    {
                        std::unique_lock<std::mutex> push_lock(_push_mutex);
                        _pop_cond.wait(push_lock, [&]()
                                       { return !_push_tasks.is_empty(); });
                        _pop_tasks.reset();
                        _pop_tasks.swap(_push_tasks);
                        _push_cond.notify_all();
                    }
                    data = _pop_tasks.pop();
                }
                if (data._log_str == "" || data._sink == nullptr)
                    continue;
                _func(data);
            }
        }

    private:
        func_t _func;
        std::mutex _push_mutex;
        std::mutex _pop_mutex;
        std::condition_variable _push_cond;
        std::condition_variable _pop_cond;
        Buffer _push_tasks;
        Buffer _pop_tasks;
        std::vector<std::thread> _threads;
    };
}
#endif