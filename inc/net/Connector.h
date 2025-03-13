#ifndef INET_CONNECTOR_H
#define INET_CONNECTOR_H

#include <functional>
#include <memory>
#include "EventLoop.h"
#include "InetAddress.h"

namespace inet
{
    class Connector
    {
    public:
        using NewConnectionCallback = std::function<void(int sockfd)>;

        enum class States
        {
            kDisconnected,
            kConnecting,
            kConnected
        };

        Connector(EventLoop *loop, const InetAddress &serverAddr);
        ~Connector();
        void setNewConnectionCAllback(const NewConnectionCallback &cb) { m_newConnectionCallback = cb; }
        void start();
        void restart();
        void stop();

        const InetAddress &getServerAddress() const { return m_serverAddr; }

    private:
        EventLoop *m_loop;
        InetAddress m_serverAddr;
        States m_state;
        std::unique_ptr<Channel> m_channel;
        bool m_connect;
        NewConnectionCallback m_newConnectionCallback;

        int m_retryDelayMs;
        static const int kMaxRetryDelayMs = 30 * 1000;
        static const int kInitRetryDelayMs = 500;

        void setState(States s) { m_state = s; }
        void startInLoop();
        void stopInLoop();
        void connect();
        void connecting(int sockfd);
        void handleWrite();
        void handleError();
        void retry(int sockfd);
        int removeAndResetChannel();
        void resetChannel();
    };
} // namespace inet

#endif