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
#define DEFAULT_ASYN_THREAD_SIZE 2 // 默认的异步工作线程池中的工作线程数量
    // 异步工作线程池模块，采用双缓冲区的思想实现,设计为单例，将来所有的Asynlogger共用同一套异步工作线程池，已保证其提供的所有操作的线程安全
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
        // 向线程池的缓冲区中放入日志数据，将来让异步工作线程读取并处理
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
        // 获取线程池的单例对象,要传入回调函数func和要创建的线程数量,但这两个参数只有全局内第一次调用get_instance()时才会用上
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
        // 异步工作线程的运行函数
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
        func_t _func;                       // 回调函数(其作用是告知异步工作线程如何处理读取上来的日志数据)
        std::mutex _push_mutex;             // 互斥锁，保证_push_tasks缓冲区的线程安全
        std::mutex _pop_mutex;              // 互斥锁，保证_pop_tasks缓冲区的线程安全
        std::condition_variable _push_cond; // 条件变量，不满足放入数据条件时外部线程就在该条件变量下等待
        std::condition_variable _pop_cond;  // 条件变量，不满足读取数据条件时异步工作线程就在该条件变量下等待
        Buffer _push_tasks;                 // 外部线程放入数据的缓冲区
        Buffer _pop_tasks;                  // 异步工作线程读取数据的缓冲区
        std::vector<std::thread> _threads;  // 管理所有创建的异步工作线程的数组
    };
}
#endif