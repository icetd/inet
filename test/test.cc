#include "Logger.h"

using namespace inet;

int main()
{      
    Logger::setLogLevel(Logger::LogLevel::INFO);
    Logger::setLogFileName("./icetd_server");

    for (;;) {
        LOG_INFO << "test";
        sleep(1);
    }   
    return 0;
}
