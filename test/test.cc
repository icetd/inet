#include "EventLoop.h"
#include "ThreadPool.h"
#include "TcpServer.h"
#include "Logger.h"

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
    LOG_INFO << buf->retrieveAllAsString();
    conn->send(buf);
}

int main()
{
    Logger::setLogLevel(Logger::LogLevel::WARN);
    Logger::setLogFileName("./server");

    LOG_WARN << "test";

    EventLoop loop;
	TcpServer server(&loop, InetAddress(8888));
    server.setConnectionCallback(c_connection);
    server.setMessageCallback(c_message);
    server.setWriteCompleteCallback(c_weite_complete);
    server.start(4);
	loop.loop();

    return 0;
}
