#include <iostream>
#include "level.hpp"
#include "sink.hpp"
#include "util.hpp"

using namespace std;

int main()
{
    log_system::LogSink::ptr stdoutptr = log_system::SinkFactory::create<log_system::StdoutSink>();

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