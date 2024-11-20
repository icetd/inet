#include "EventLoop.h"
#include "ThreadPool.h"
#include "TcpClient.h"
#include "Logger.h"
#include <iostream>

using namespace inet;

void c_connection(const TcpConnectionPtr &conn)
{
    LOG_INFO << "new connect";
}

void c_weite_complete(const TcpConnectionPtr &coon)
{
    LOG_INFO << "write complete";
}

void c_message(const TcpConnectionPtr &conn, Buffer *buf)
{
    conn->send("hello");
}

int main()
{
    Logger::setLogLevel(Logger::LogLevel::WARN);
    Logger::setLogFileName("./server");

    EventLoop loop;

    TcpClient client(&loop, InetAddress(6666, "192.168.2.87"));

    client.setConnectionCallback(c_connection);
    client.setMessageCallback(c_message);
    client.setWriteCompleteCallback(c_weite_complete);
    client.connect();
    loop.loop();
    return 0;
}
