#include <iostream>
#include <cassert>
#include "level.hpp"
#include "sink.hpp"
#include "util.hpp"

using namespace std;

int main()
{
    log_system::LogSink::ptr Stdoutptr = log_system::SinkFactory::create<log_system::StdoutSink>();
    log_system::LogSink::ptr Fileptr = log_system::SinkFactory::create<log_system::FileSink>("./data/file.log");
    log_system::LogSink::ptr RollFileSinkBySizeptr = log_system::SinkFactory::create<log_system::RollFileSinkBySize>("./data/file.log", 1024 * 200);
    assert(Stdoutptr->log("abcdefg\n"));
    assert(Fileptr->log("abcdefg\n"));
    for (int i = 0; i < 100000; i++)
    {
        assert(RollFileSinkBySizeptr->log(to_string(i) + "abcdefg\n"));
        usleep(100);
    }

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