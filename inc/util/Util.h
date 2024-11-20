#ifndef INET_UTIL_H
#define INET_UTIL_H

#include <string>
#include "Platform.h"

namespace inet
{
    namespace sockets
    {
        void setReuseAddr(int sockfd);
        void setNonblock(int sockfd);
        void shutdownWrite(int sockfd);
        int getSocketError(int sockfd);

        int createNonblockingOrDie();
        int connect(int sockfd, const struct sockaddr_in* addr);

        bool isSelfConnect(int sockfd);
        void close(int sockfd);

        struct sockaddr_in getLocalAddr(int sockfd);
        struct sockaddr_in getPeerAddr(int sockfd);
    } // namespace sockets

    namespace ProcessInfo
    {
        std::string hostname();
        pid_t pid();
    }
    void perror_if(bool condtion, const char *errorMessage);
}

#endif