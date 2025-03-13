#include <iostream>
#include "StaticWebServer.h"
#include "EventLoop.h"
#include "Logger.h"
#include "INIReader.h"

using namespace inet;

int main(int argc, char *argv[])
{
    int port = 0;
    int thread_num = 0;
    int log_level = 0;
    std::string log_basename = {};
    std::string root_path = {};

    INIReader configs("./configs/config.ini");
    if (configs.ParseError() < 0) {
        printf("read config failed.");
        exit(1);
    } else {
        log_level = configs.GetInteger("log", "level", 3);
        log_basename = configs.Get("log", "basename", "./default");
        port = configs.GetInteger("server", "port", 8000);
        thread_num = configs.GetInteger("server", "threadnum", 1);
        root_path = configs.Get("server", "rootpath", "./www");
    }

    Logger::setLogFileName(log_basename);
    Logger::setLogLevel(static_cast<Logger::LogLevel>(log_level));

    EventLoop loop;
    StaticWebServer server(&loop, port);
    server.setRootDirectory(root_path);
    server.start(thread_num);

    std::cout << "Server start on: " << port << "\n"
              << "Server thread num: " << thread_num << "\n"
              << "Server root path: " << root_path << "\n"
              << "Logger basename: " << log_basename << "\n"
              << "Logger level: " << Logger::getGlobalLogLevel() << "\n";

    loop.loop();
    return 0;
}