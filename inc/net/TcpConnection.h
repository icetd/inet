#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <memory>

#include "InetAddress.h"
#include "Callbacks.h"
#include "Socket.h"
#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"

namespace inet
{
    class Channel;

    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {
    public:
        enum class StateE {
            kDisconnected,
            kConnecting,
            kConnected,
            kDisconnecting
        };

    public:
        TcpConnection(EventLoop *loop, int sockfd, const InetAddress &loaclAddr, const InetAddress &peerAddr);
        ~TcpConnection();

        void setMessageCallback(const MessageCallback &cb)
        {
            m_messageCallback = cb;
        }

        void setCloseCallback(const CloseCallback &cb)
        {
            m_closeCallback = cb;
        }

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            m_connectionCallback = cb;
        }

        void setWriteCompleteCallback(const WriteCompleteCallback &cb)
        {
            m_writeCompleteCallback = cb;
        }

        const InetAddress &localAddress() const { return m_localAddr; }
        const InetAddress &peerAddress() const { return m_peerAddr; }

        bool connected() const { return m_state == StateE::kConnected; }
        bool disconnected() const { return m_state == StateE::kDisconnected; }
        void send(Buffer *message);
        void send(const char *message, size_t len);
        void send(const std::string &messgage);

        void shutdown();

        void forceClose();

        void connectEstablished();

        void connectDestroyed();

        // Advanced interface
        Buffer *inputBuffer()
        {
            return &m_inputBuffer;
        }

        Buffer *outputBuffer()
        {
            return &m_outputBuffer;
        }
        EventLoop *getLoop() const { return m_loop; }

        int fd() const { return m_socket->fd(); }

    private:
        void handleRead();
        void handleWrite();
        void handleClose();
        void handleError();

        void shutdownInLoop();
        void forceCloseInLoop();
        void sendInLoop(const char *message, size_t len);

        void setState(StateE state) { m_state = state; }

    private:
        EventLoop *m_loop;

        StateE m_state; // FIXME: use atomic variable

        std::unique_ptr<Socket> m_socket;
        std::unique_ptr<Channel> m_channel;

        const InetAddress m_localAddr;
        const InetAddress m_peerAddr;

        CloseCallback m_closeCallback;
        ConnectionCallback m_connectionCallback;
        MessageCallback m_messageCallback;
        WriteCompleteCallback m_writeCompleteCallback;

        Buffer m_inputBuffer;
        Buffer m_outputBuffer;
    };
}

#endif

