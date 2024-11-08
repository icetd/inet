#include<string.h>
#include<unistd.h>
#include<fcntl.h>

#include "TcpServer.h"
#include "Util.h"

using namespace inet;

TcpServer::TcpServer(EventLoop *eventloop, const InetAddress &listenAddr) :
    m_loop(eventloop),
    m_ipPort(listenAddr.toIpPort()),
    m_acceptor(std::make_unique<Acceptor>(m_loop, listenAddr)),
    m_evloop_threadpool(std::make_unique<EventLoopThreadPool>(m_loop)),
    m_compute_threadpool(std::make_unique<ThreadPool>()),
    m_started(0)
{
    m_acceptor->setNewconnectionCallback([this](int sockfd, const InetAddress &peerAddr) { 
        newConnection(sockfd, peerAddr);
    });
}

TcpServer::~TcpServer()
{
    // 不一定会进入到下面的for循环中的，因为可能m_connections中已没有元素
    for (auto &item : m_connections) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->connectDestroyed();
    }
}

void TcpServer::start(int IOThreadNum, int compute_threadNum)
{
    if (m_started++ == 0) // 防止一个TcpServer对象被启动多次
    {
        m_evloop_threadpool->setThreadNum(IOThreadNum);
        m_evloop_threadpool->start();
        m_compute_threadpool->start(compute_threadNum); // 添加这句，开启计算线程池，若compute_threadNum是0，那就是没有开启

        m_acceptor->listen();
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop *ioLoop = m_evloop_threadpool->getNextLoop();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    auto conn = std::make_shared<TcpConnection>(ioLoop, sockfd, localAddr, peerAddr);
    m_connections[sockfd] = conn;

    conn->setMessageCallback(m_messageCallback);
    conn->setCloseCallback([this](const TcpConnectionPtr &connection) { 
        removeConnection(connection); });
    conn->setConnectionCallback(m_connectionCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);

    ioLoop->runInLoop([conn]() { conn->connectEstablished(); });
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    m_loop->runInLoop([this, conn]() { removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    m_connections.erase(conn->fd());

    auto ioLoop = conn->getLoop();
    ioLoop->queueInLoop([conn]() { conn->connectDestroyed(); });
}
