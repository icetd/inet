#ifndef INET_SOCKET_H
#define INET_SOCKET_H

#include <string>
#include "Platform.h"
#include "InetAddress.h"

namespace inet 
{
    class Socket
    {
    public:
        explicit Socket(int sockfd) : m_sockfd(sockfd) {}
        ~Socket();

        void bind(const InetAddress &localaddr);
        void listen() const;
        int accept(InetAddress *peeraddr);
        void setNonblock();

        int fd() { return m_sockfd; }

    private:
        int m_sockfd;
    };
} // namespace net

#endif