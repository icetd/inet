#ifndef INET_EPOLL_H
#define INET_EPOLL_H

#include <vector>
#include "Platform.h"

namespace inet
{   
    class Channel;
    class Epoll
    {
    public:
        Epoll();
        ~Epoll();

        void updateChannel(Channel *ch);
        void destroy(Channel *ch);
        int getEpollFd() const { return m_epfd; }
        void epollWait(std::vector<Channel*> &active, int timeout = 10);

    private:
        int m_epfd;
        struct epoll_event *m_events;
    };
}


#endif