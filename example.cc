#include <iostream>
#include <cassert>
#include "log.h"

using namespace std;

void test(const log_system::Logger::ptr &logger)
{
    LOG_DEBUG(logger, "日志输出测试-DEBUG");
    LOG_INFO(logger, "日志输出测试-INFO");
    LOG_WARN(logger, "日志输出测试-WARN");
    LOG_ERROR(logger, "日志输出测试-ERROR");
    LOG_FATAL(logger, "日志输出测试-FATAL");
    for (int i = 0; i < 20000; i++)
        LOG(logger, log_system::Level::ERROR, "日志输出测试-%d", i);
}

void test1()
{
    std::vector<log_system::LogSink::ptr> asynsinks1;
    asynsinks1.push_back(log_system::get_sink<log_system::StdoutSink>());
    asynsinks1.push_back(log_system::get_sink<log_system::FileSink>("./data/file.log"));
    asynsinks1.push_back(log_system::get_sink<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 1024 * 4));
    std::vector<log_system::LogSink::ptr> asynsinks2;
    asynsinks2.push_back(log_system::get_sink<log_system::StdoutSink>());
    asynsinks2.push_back(log_system::get_sink<log_system::FileSink>("./data/file.log"));
    asynsinks2.push_back(log_system::get_sink<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 1024 * 4));
    std::vector<log_system::LogSink::ptr> asynsinks3;
    asynsinks3.push_back(log_system::get_sink<log_system::StdoutSink>());
    asynsinks3.push_back(log_system::get_sink<log_system::FileSink>("./data/file.log"));
    asynsinks3.push_back(log_system::get_sink<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 1024 * 4));

    log_system::add_logger("asynlogger1", log_system::ASYNC_LOGGER, asynsinks1, log_system::Level::ERROR);
    log_system::add_logger("asynlogger2", log_system::ASYNC_LOGGER, asynsinks2, log_system::Level::ERROR);
    log_system::add_logger("asynlogger3", log_system::ASYNC_LOGGER, asynsinks3, log_system::Level::ERROR);

    log_system::Logger::ptr asynlogger1 = log_system::get_logger("asynlogger1");
    log_system::Logger::ptr asynlogger2 = log_system::get_logger("asynlogger2");
    log_system::Logger::ptr asynlogger3 = log_system::get_logger("asynlogger3");

    std::thread t1(test, asynlogger1);
    std::thread t2(test, asynlogger2);
    std::thread t3(test, asynlogger3);
    t1.join();
    t2.join();
    t3.join();
}

void test2()
{
    std::vector<log_system::LogSink::ptr> asynsinks;
    asynsinks.push_back(log_system::get_sink<log_system::StdoutSink>());
    asynsinks.push_back(log_system::get_sink<log_system::FileSink>("./data/file.log"));
    asynsinks.push_back(log_system::get_sink<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 1024 * 4));
    std::vector<log_system::LogSink::ptr> synsinks;
    synsinks.push_back(log_system::get_sink<log_system::StdoutSink>());
    synsinks.push_back(log_system::get_sink<log_system::FileSink>("./data/file.log"));
    synsinks.push_back(log_system::get_sink<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 1024 * 4));

    log_system::add_logger("asynlogger", log_system::ASYNC_LOGGER, asynsinks, log_system::Level::ERROR);
    log_system::add_logger("synlogger", log_system::SYNC_LOGGER, synsinks, log_system::Level::ERROR);

    log_system::Logger::ptr asynlogger = log_system::get_logger("asynlogger");
    log_system::Logger::ptr synlogger = log_system::get_logger("synlogger");

    std::thread t1(test, asynlogger);
    std::thread t2(test, asynlogger);
    std::thread t3(test, synlogger);
    std::thread t4(test, synlogger);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

void test3()
{
    log_system::Logger::ptr rootlogger = log_system::get_logger("root");
    LOG_DEBUG(rootlogger, "日志输出测试-DEBUG");
    LOG_INFO(rootlogger, "日志输出测试-INFO");
    LOG_WARN(rootlogger, "日志输出测试-WARN");
    LOG_ERROR(rootlogger, "日志输出测试-ERROR");
    LOG_FATAL(rootlogger, "日志输出测试-FATAL");
    for (int i = 0; i < 5; i++)
        LOG(rootlogger, log_system::Level::WARN, "日志输出测试-%d", i);
}

int main()
{
    // test1();
    // test2();
    test3();
    return 0;
}