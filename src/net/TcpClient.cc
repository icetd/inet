#include "TcpClient.h"
#include "Logger.h"
#include "Util.h"

using namespace inet;

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr) :
    m_loop(loop),
    m_connector(std::make_shared<Connector>(loop, serverAddr)),
    m_retry(false),
    m_connect(true),
    m_nextConnId(1)
{
    m_connector->setNewConnectionCAllback([this](int sockfd) { newConnection(sockfd); });
    LOG_INFO << "TcpClient::TcpClient";
}

TcpClient::~TcpClient()
{
    bool unique = false;
    TcpConnectionPtr conn;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        unique = m_connection.unique();
        conn = m_connection;
    }

    if (conn) {
        assert(m_loop == conn->getLoop());
        m_loop->queueInLoop([conn](){ conn->connectDestroyed(); });
        if (unique) {
            conn->forceClose();
        }
    } else {
        m_connector->stop();
    }
}

void TcpClient::connect()
{
    m_connect = true;
    m_connector->start();
}

void TcpClient::disconnect()
{
    m_connect = false;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_connection) {
            m_connection->shutdown();
        }
    }
}

void TcpClient::stop()
{
    m_connect = false;
    m_connector->stop();
}

void TcpClient::newConnection(int sockfd)
{
    m_loop->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(m_loop, sockfd, localAddr, peerAddr));

    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback([this](const TcpConnectionPtr &connection)
                           { removeConnection(connection); });

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connection = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn)
{
    m_loop->runInLoop([this, conn]() { removeConnectionInLoop(conn); });
}


void TcpClient::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    auto ioLoop = conn->getLoop();
    ioLoop->queueInLoop([conn]() { conn->connectDestroyed(); });
}