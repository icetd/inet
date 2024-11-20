#ifndef INET_TCPCLIENT_H
#define INET_TCPCLIENT_H

#include <mutex>
#include "TcpConnection.h"
#include "Connector.h"
#include "EventLoop.h"

namespace inet
{
    class TcpClient
    {
    public:
        using ConnectorPrt = std::shared_ptr<Connector>;

        TcpClient(EventLoop *loop, const InetAddress &serverAddr);
        ~TcpClient();

        void connect();
        void disconnect();
        void stop();

        TcpConnectionPtr connection()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_connection;
        }

        EventLoop *getLoop() const { return m_loop; }
        bool retry() const { return m_retry; }
        void enableRetry() { m_retry = true; }
        bool isConnected() { return m_connect; }

        void setConnectionCallback(ConnectionCallback cb) { m_connectionCallback = std::move(cb); }
        void setMessageCallback(MessageCallback cb) { m_messageCallback = std::move(cb); }
        void setWriteCompleteCallback(WriteCompleteCallback cb) { m_writeCompleteCallback = std::move(cb); }


    private:
        EventLoop *m_loop;

        std::mutex m_mutex;

        ConnectionCallback m_connectionCallback;
        MessageCallback m_messageCallback;
        WriteCompleteCallback m_writeCompleteCallback;

        bool m_retry;
        bool m_connect;
        int m_nextConnId;
        ConnectorPrt m_connector;
        TcpConnectionPtr m_connection;

        void newConnection(int sockfd);
        void removeConnection(const TcpConnectionPtr &conn);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);
    };
}
#endif