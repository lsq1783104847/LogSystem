#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include "level.hpp"
#include "sink.hpp"
#include "format_message.hpp"
#include "logger.hpp"
#include "buffer.hpp"
#include "util.hpp"

using namespace std;

void test(const log_system::Logger::ptr &logger)
{
    logger->log(log_system::Level::DEBUG, __FILE__, __LINE__, "日志输出测试-DEBUG");
    logger->log(log_system::Level::INFO, __FILE__, __LINE__, "日志输出测试-INFO");
    logger->log(log_system::Level::WARN, __FILE__, __LINE__, "日志输出测试-WARN");
    logger->log(log_system::Level::ERROR, __FILE__, __LINE__, "日志输出测试-ERROR");
    logger->log(log_system::Level::FATAL, __FILE__, __LINE__, "日志输出测试-FATAL");
    for (int i = 0; i < 50000; i++)
    {
        logger->log(log_system::Level::ERROR, __FILE__, __LINE__, "日志输出测试-%d", i);
        usleep(100);
    }
}

int main()
{
    log_system::LogSink::ptr Stdoutptr = log_system::SinkFactory::create<log_system::StdoutSink>();
    log_system::LogSink::ptr Fileptr = log_system::SinkFactory::create<log_system::FileSink>("./data/file.log");
    log_system::LogSink::ptr RollFileSinkBySizeptr = log_system::SinkFactory::create<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 1024);
    std::vector<log_system::LogSink::ptr> sinks = {Stdoutptr, Fileptr, RollFileSinkBySizeptr};
    log_system::Logger::ptr logger(new log_system::AsynLogger("asynlogger", {Stdoutptr, Fileptr, RollFileSinkBySizeptr}, log_system::Level::WARN));

    // log_system::LogSink::ptr Stdoutptr = log_system::SinkFactory::create<log_system::StdoutSink>();
    // log_system::LogSink::ptr Fileptr = log_system::SinkFactory::create<log_system::FileSink>("./data/file.log");
    // log_system::LogSink::ptr RollFileSinkBySizeptr = log_system::SinkFactory::create<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 1024);
    // std::vector<log_system::LogSink::ptr> sinks = {Stdoutptr, Fileptr, RollFileSinkBySizeptr};
    // log_system::Logger::ptr logger(new log_system::SynLogger("synlogger", {Stdoutptr, Fileptr, RollFileSinkBySizeptr}, log_system::Level::WARN));

    std::thread t1(test, logger);
    std::thread t2(test, logger);

    // logger->log(log_system::Level::DEBUG, __FILE__, __LINE__, "日志输出测试-DEBUG");
    // logger->log(log_system::Level::INFO, __FILE__, __LINE__, "日志输出测试-INFO");
    // logger->log(log_system::Level::WARN, __FILE__, __LINE__, "日志输出测试-WARN");
    // logger->log(log_system::Level::ERROR, __FILE__, __LINE__, "日志输出测试-ERROR");
    // logger->log(log_system::Level::FATAL, __FILE__, __LINE__, "日志输出测试-FATAL");
    // for (int i = 0; i < 50000; i++)
    // {
    //     logger->log(log_system::Level::ERROR, __FILE__, __LINE__, "日志输出测试-%d", i);
    //     usleep(100);
    // }

    // log_system::LogMsg lm(__FILE__, __LINE__, time(nullptr), std::this_thread::get_id(), "lsq", "日志测试", log_system::Level::FATAL);
    // std::cout << log_system::LogFmt("%t[%D_%D_%T_%T]%n[%i_%L_%N_%f_%l_%m_%%]", lm).get_result() << std::endl;

    // log_system::LogSink::ptr Stdoutptr = log_system::SinkFactory::create<log_system::StdoutSink>();
    // log_system::LogSink::ptr Fileptr = log_system::SinkFactory::create<log_system::FileSink>("./data/file.log");
    // log_system::LogSink::ptr RollFileSinkBySizeptr = log_system::SinkFactory::create<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 200);
    // assert(Stdoutptr->log("abcdefg\n"));
    // assert(Fileptr->log("abcdefg\n"));
    // for (int i = 0; i < 100000; i++)
    // {
    //     assert(RollFileSinkBySizeptr->log(to_string(i) + "abcdefg\n"));
    //     usleep(100);
    // }

    // cout << log_system::Util::get_fname("fawfaw") << endl;
    // cout << log_system::Util::get_fname("/fasdfasf/hrjrj/qewtqet/") << endl;
    // cout << log_system::Util::get_fname("/") << endl;
    // cout << log_system::Util::get_fname("./abcd/1234/6666") << endl;

    // log_system::Level::value val = log_system::Level::DEBUG;
    // cout << log_system::Level::toString(val) << endl;
    // val = log_system::Level::INFO;
    // cout << log_system::Level::toString(val) << endl;
    // val = log_system::Level::WARN;
    // cout << log_system::Level::toString(val) << endl;
    // val = log_system::Level::ERROR;
    // cout << log_system::Level::toString(val) << endl;
    // val = log_system::Level::FATAL;
    // cout << log_system::Level::toString(val) << endl;

    return 0;
}