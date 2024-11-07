#include <string.h>
#include "Epoll.h"

using namespace inet;

const int kSize = 1024;

Epoll::Epoll() :
    m_epfd(epoll_create(1)),
    m_events(new epoll_event[kSize])
{
    if (m_epfd == -1)
        perror("epoll_create");
    memset(m_events, 0, sizeof(kSize * sizeof(epoll_event)));
}

Epoll::~Epoll()
{
    if (m_epfd != -1) {
        m_epfd = -1;
    }

    delete m_events;
}

void Epoll::updateChannel(Channel *ch)
{
    int fd = ch->getFd();
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.ptr = ch; // epoll_data_t is user data
    ev.events = ch->getEevents();

    if (ch->isInEpoll()) {
        if (ch->isNoneEvent()) {  //channel is already not listen event
            destroy(ch);
        } else {
            int ret = epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
            if (ret == -1)
                perror("epoll_ctl mod");
        }
    } else {
        int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
        if (ret == -1) {
            perror("epoll_ctl add");
            ch->setInEopll(true);
        }
    }
}

void Epoll::destroy(Channel *ch)
{
    if (ch->isInEpoll()) {
        int ret = epoll_ctl(m_epfd, EPOLL_CTL_DEL, ch->getFd(), nullptr);
        if (ret == -1) {
            perror("epoll_ctl del");
            ch->setInEopll(false);
        }
    }
}

void Epoll::epollWait(std::vector<Channel*> &active, int timeout)
{
    int nfds = epoll_wait(m_epfd, m_events, kSize, timeout);
    if (nfds == -1)
        perror("epoll_wait");
    for (int i = 0; i < nfds; i++) {
        Channel *ch = static_cast<Channel*> (m_events[i].data.ptr);
        ch->setRevents(m_events[i].events); // set channel return event from epoll
        active.emplace_back(ch);
    }
}