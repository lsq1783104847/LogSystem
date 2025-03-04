// 该文件主要是进行当前环境下的同步/异步日志器向指定文件输出时的极限性能测试

#include "log.h"
#include <chrono>
#include <stdlib.h>

// 构造log_len长度的日志，将log_size条日志平均分发给thread_size个线程通过logger_name日志器输出，并计算耗时
void test(const std::string &logger_name, size_t thread_size, size_t log_size, size_t log_len)
{
    // 对传入参数进行条件判断
    if (thread_size <= 0 || log_size <= 0 || log_len <= 0)
        return;
    // 根据名称获取日志器
    log_system::Logger::ptr logger = log_system::get_logger(logger_name);
    if (logger == nullptr)
        return;
    // 构建指定长度的日志字符串，且最后一个字符为'\n'
    std::string str(log_len - 1, 'L');
    str.push_back('\n');
    // 计算每个线程要输出日志的数量并保存
    std::vector<size_t> thread_log_size;
    for (size_t i = 0; i < thread_size; i++)
        thread_log_size.emplace_back(log_size / thread_size);
    thread_log_size[0] += log_size % thread_size;
    // 用于保存将来的总耗时
    double all_cost = 0;
    // 创建指定数量的线程
    std::vector<std::thread> threads;
    auto all_start = std::chrono::high_resolution_clock::now(); // 总计时开始
    for (size_t i = 0; i < thread_size; i++)
    {
        threads.emplace_back([&, i]()
                             {
            auto start = std::chrono::high_resolution_clock::now(); // 单线程计时开始
            // 让线程输出指定条数的日志
            for (size_t j = 0; j < thread_log_size[i]; j++)
                LOG_DEBUG(logger, str.c_str());
            auto end = std::chrono::high_resolution_clock::now(); // 单线程计时结束
            std::chrono::duration<double> time = end - start;     // 计算单线程耗时
            std::chrono::duration<double> all_time = end-all_start;
            all_cost = (all_cost > all_time.count()? all_cost:all_time.count()); // 计算总耗时
            std::cout << "线程" << i+1 << "输出：" << thread_log_size[i] << "条日志\t耗时: " << time.count() << "s\n"; });
    }
    // 回收线程
    for (auto &thread : threads)
        thread.join();

    std::cout << "总输出日志条数: " << (size_t)log_size << "条" << std::endl;
    std::cout << "总输出日志大小: " << (size_t)(log_size * log_len / 1024 / 1024) << "MB" << std::endl;
    std::cout << "总消耗时间: " << all_cost << "s\n";
    std::cout << "平均每秒输出日志条数: " << (size_t)(log_size / all_cost) << "条" << std::endl;
    std::cout << "平均每秒输出日志大小: " << (size_t)(log_size * log_len / all_cost / 1024 / 1024) << "MB" << std::endl;
}
// 同步日志器性能测试
void syn_test()
{
    log_system::add_logger("SynLogger", log_system::SYNC_LOGGER, {log_system::get_sink<log_system::FileSink>("~/data/file.log")}, log_system::Level::DEBUG, "%m");
    // test("SynLogger", 1, 1000000, 100); // 单线程输出
    test("SynLogger", 3, 1000000, 100); // 多线程输出
}
// 异步日志器性能测试
// （对于异步日志器只计算日志输出到异步线程池所提供的缓冲区中的时间，并不计算实际落入外设中的时间）
void asyn_test()
{
    log_system::add_logger("AsynLogger", log_system::ASYNC_LOGGER, {log_system::get_sink<log_system::FileSink>("~/data/file.log")}, log_system::Level::DEBUG, "%m");
    // test("AsynLogger", 1, 1000000, 100); // 单线程输出
    test("AsynLogger", 3, 1000000, 100); // 多线程输出
}

int main()
{
    // syn_test();
    asyn_test();
    return 0;
}
