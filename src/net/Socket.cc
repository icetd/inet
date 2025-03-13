#include "Socket.h"

using namespace inet;

Socket::Socket() :
    m_sockfd(socket(AF_INET, SOCK_STREAM, 0))
{
    if (m_sockfd == -1)
        perror("scoket");
}

Socket::~Socket()
{
    if (m_sockfd != -1) {
        close(m_sockfd);
        m_sockfd = -1;
    }
}

void Socket::bind(const InetAddress &localaddr)
{
    int ret = ::bind(m_sockfd, (struct sockaddr *)localaddr.getSockAddr(), sizeof(sockaddr_in));
    if (ret == -1) {
        perror("bind");
    }
}

void Socket::listen() const
{
    int ret = ::listen(m_sockfd, 128);
    if (ret == -1) {
        perror("listen");
    }
}

int Socket::accept(InetAddress *peeraddr)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = ::accept4(m_sockfd, (sockaddr *)&client_addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (client_fd == -1) {
        perror("accept");
    }
    return client_fd;
}

void Socket::setNonblock()
{
    int flag = fcntl(m_sockfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(m_sockfd, F_SETFL, flag);
}