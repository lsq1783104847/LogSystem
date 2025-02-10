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
    for (int i = 0; i < 30000; i++)
    {
        int ret = logger->log(log_system::Level::ERROR, __FILE__, __LINE__, "日志输出测试-%d", i);
        assert(ret != -1);
    }
}

int main()
{

    // std::string path = "/home/lsq/log_system/log.txt";
    // std::ofstream _ofs;
    // _ofs.open(path, std::ios::binary | std::ios::app);
    // std::cout << (_ofs.is_open() == true ? "TRUE" : "FALSE") << std::endl;

    std::cout << ((std::shared_ptr<log_system::Logger>() == nullptr) ? "True" : "False") << std::endl;
    std::cout << ((std::shared_ptr<log_system::Logger>(nullptr) == nullptr) ? "True" : "False") << std::endl;

    // std::string path1 = "./data/..";
    // std::string path2 = "/home/lsq/linux_code/abcd/edgda/";
    // std::string path3 = "./../../../../../datagfawg/gawga";
    // std::string ret1 = log_system::Util::path_transform(path1);
    // std::string ret2 = log_system::Util::path_transform(path2);
    // std::string ret3 = log_system::Util::path_transform(path3);
    // std::cout << "path1:" << path1 << std::endl;
    // std::cout << "path2:" << path2 << std::endl;
    // std::cout << "path3:" << path3 << std::endl;
    // std::cout << "ret1:" << ret1 << std::endl;
    // std::cout << "ret2:" << ret2 << std::endl;
    // std::cout << "ret3:" << ret3 << std::endl;

    log_system::LogSink::ptr Stdoutptr = log_system::StdoutSink::get_sink();
    log_system::LogSink::ptr Fileptr = log_system::FileSink::get_sink("./data/file.log");
    log_system::LogSink::ptr RollFileSinkBySizeptr = log_system::RollFileSinkBySize::get_sink("./data/file.log", 1024 * 1024 * 4);
    std::vector<log_system::LogSink::ptr> sinks = {Stdoutptr, Fileptr, RollFileSinkBySizeptr};

    log_system::Logger::ptr asynlogger(new log_system::SynLogger("synlogger", sinks, log_system::Level::WARN));
    log_system::Logger::ptr logger(new log_system::AsynLogger("asynlogger", sinks, log_system::Level::WARN));

    std::thread t1(test, logger);
    std::thread t2(test, logger);
    std::thread t3(test, asynlogger);
    std::thread t4(test, asynlogger);
    t1.join();
    t2.join();
    t3.join();
    t4.join();

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