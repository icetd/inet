#include <functional>
#include "InetAddress.h"

#include "Socket.h"
#include "Channel.h"

namespace inet
{
    class EventLoop;

    class Acceptor
    {
    public:
        using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;

    public:
        Acceptor(EventLoop *eventloop, const InetAddress &listenAddr);
        ~Acceptor();

        void setNewconnectionCallback(const NewConnectionCallback &cb) { m_newConnectionCallback = cb; }

        void listen();

    private:
        void handleRead();

        EventLoop *m_loop;
        Socket m_acceptSocket;
        Channel m_acceptChannel;

        NewConnectionCallback m_newConnectionCallback;
        bool m_listen;
    };

} // namespace inet