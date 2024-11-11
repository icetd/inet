#include "StaticWebServer.h"
#include "EventLoop.h"
#include "Logger.h"

int main(int argc, char* argv[]) {
    
    Logger::setLogFileName("./server");
    Logger::setLogLevel(Logger::LogLevel::WARN);
    LOG_WARN << "test log";
    
    EventLoop loop;
    StaticWebServer server(&loop, 9999);
    server.setRootDirectory("./www");
    server.start(4);
    loop.loop();

    return 0;
}