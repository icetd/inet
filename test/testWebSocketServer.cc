#include "WebSocketServer.h"
#include "Logger.h"
#include <iostream>
#include <unistd.h>

using namespace inet;

int64_t sendTaskId = -1;

void sendMessage(const TcpConnectionPtr &conn)
{
    if (conn && conn->connected()) {
        WebSocketServer::getInstance()->send("Hello", 6, WSCodeType::WSCodeText, conn);
    }
}

void onRequest(const Buffer *input, const TcpConnectionPtr &conn)
{
    std::string cmd(input->peek(), input->readableBytes());
    std::cout << cmd << std::endl;
    if (cmd == "start") {
        EventLoop *loop = conn->getLoop();
        sendTaskId = loop->runEvery(0.01, [conn]() { sendMessage(conn); });
    } else if (cmd == "stop") {
        EventLoop *loop = conn->getLoop();
        loop->cancel(sendTaskId);
    }
}

int main()
{
    int numThreads = 5;
    Logger::setLogLevel(Logger::LogLevel::WARN);
    Logger::setOutput(DefaultOutput);
    EventLoop loop;
    WebSocketServer server(&loop, InetAddress(9000));
    server.setHttpCallback(onRequest);
    server.start(numThreads);
    LOG_INFO << "start";
    loop.loop();
    return 0;
}
