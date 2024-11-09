#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

using namespace inet;

Channel::Channel(EventLoop *loop, int fd) : 
    m_loop(loop),
    m_fd(fd),
    m_events(0),
    m_revents(0),
    m_isInEpoll(false),
    m_tied(false)
{
}

void Channel::setEvents(int events)
{
    m_events = events;
}

int Channel::getEevents() const
{
    return m_events;
}

void Channel::setRevents(int revents)
{
    m_revents = revents;
}

int Channel::getRevents() const
{
    return m_revents;
}

bool Channel::isInEpoll()
{
    return m_isInEpoll == true;
}

void Channel::setInEopll(bool in)
{
    m_isInEpoll = in;
}

int Channel::getFd() const
{
    return m_fd;
}

void Channel::handleEvent()
{
    if (m_tied) {
        std::shared_ptr<void> guard = m_tie.lock();
        if (guard)
            handleEventWithGurad(); // 连接已经建立，可以安全地处理事件
    } else {
        // 连接尚未建立，直接进行连接建立或初始化操作, 在建立连接后会将 tied_ 设置为 true
        handleEventWithGurad();
    }
}

void Channel::remove()
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    m_tie = obj;
    m_tied = true;
}

void Channel::update()
{
    m_loop->updateChannel(this);
}

void Channel::handleEventWithGurad()
{
    // LOG_INFO << reventsToString();

    if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN)) { // hup and no read
        if (m_closeCallback) {
            LOG_DEBUG << "channel closeCallback";
            m_closeCallback();
        }
    }

    if (m_revents & EPOLLERR) { // error event
        if (m_errorCallback)
        {
            m_errorCallback();
        }
    }

    if (m_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) { // read event
        if (m_readCallback)
        {
            m_readCallback();
        }
    }

    if (m_revents & EPOLLOUT) { // write event
        if (m_writeCallback) {
            m_writeCallback();
        }
    }
}

std::string Channel::reventsToString() const
{
    return eventsToString(m_fd, m_revents);
}

std::string Channel::eventsToString() const
{
    return eventsToString(m_fd, m_events);
}

std::string Channel::eventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & EPOLLIN)
        oss << "IN ";
    if (ev & EPOLLPRI)
        oss << "PRI ";
    if (ev & EPOLLOUT)
        oss << "OUT ";
    if (ev & EPOLLHUP)
        oss << "HUP ";
    if (ev & EPOLLRDHUP)
        oss << "RDHUP ";
    if (ev & EPOLLERR)
        oss << "ERR ";

    return oss.str();
}