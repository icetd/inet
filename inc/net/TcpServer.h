#ifndef INET_TCPSERVER_H
#define INET_TCPSERVER_H

#include "Acceptor.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"
#include "ThreadPool.h"
#include <map>

namespace inet
{
    class TcpServer
    {
    public:
        using connectionMap = std::map<int, TcpConnectionPtr>;

        TcpServer(EventLoop *eventloop, const InetAddress &serverAddr);
        ~TcpServer();

        void start(int IOThreadNum = 0, int threadNum = 0);
        
        void setMessageCallback(const MessageCallback &cb)
        {
            m_messageCallback = cb;
        }

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            m_connectionCallback = cb;
        }

        void setWriteCompleteCallback(const WriteCompleteCallback &cb)
        {
            m_writeCompleteCallback = cb;
        }

        EventLoop *getLoop() const { return m_loop; }
    
    private:
        EventLoop *m_loop;

        const std::string m_ipPort;

        std::unique_ptr<Acceptor> m_acceptor;
        connectionMap m_connections;

        std::unique_ptr<EventLoopThreadPool> m_evloop_threadpool;
        std::unique_ptr<ThreadPool> m_compute_threadpool;
        std::atomic_int32_t m_started;

        WriteCompleteCallback m_writeCompleteCallback;
        MessageCallback m_messageCallback;
        ConnectionCallback m_connectionCallback;
 
        void newConnection(int sockfd, const InetAddress &peerAddr);
        void removeConnection(const TcpConnectionPtr &conn);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);
    };
}

#endif