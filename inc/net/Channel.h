#ifndef INET_CHANNEL_H
#define INET_CHANNEL_H

#include <functional>
#include <memory>
#include <sstream>

#include "Platform.h"

namespace inet
{   
    class EventLoop;
    class Channel
    {
    public:
        using EventCallback = std::function<void()>;

        Channel(EventLoop* loop, int fd);
        ~Channel() = default;

        void setEvents(int events);
        int getEevents() const; 
        void setRevents(int events);
        int getRevents() const;

        bool isInEpoll();
        void setInEopll(bool in);
        int getFd() const;

        void setReadCallback(EventCallback cb) { m_readCallback = std::move(cb); }
        void setWriteCallback(EventCallback cb) { m_writeCallback = std::move(cb); }
        void setCloseCallback(EventCallback cb) { m_closeCallback = std::move(cb); }
        void setErrorCallback(EventCallback cb) { m_errorCallback = std::move(cb); }

	    void enableReading() { m_events |= (EPOLLIN | EPOLLPRI); update(); }	//注册可读事件
	    void disableReading() { m_events &= ~(EPOLLIN | EPOLLPRI); update(); }	//注销可读事件
	    void enableWriting() { m_events |= EPOLLOUT; update(); }			    //注册可写事件
	    void disableWriting() { m_events &= ~EPOLLOUT; update(); }		        //注销可写事件
	    void disableAll() { m_events = 0; update();}	//注销所有事件

        bool isNoneEvent() const { return m_events == 0; }
        bool isWrite() const { return m_events & EPOLLOUT; }
        bool isRead() const { return m_events & (EPOLLIN | EPOLLPRI); }

        void handleEvent();
        void remove();
        void tie(const std::shared_ptr<void>&);

        // for debug
        std::string reventsToString() const;
        std::string eventsToString() const;

    private:
        int m_fd;
        int m_events;
        int m_revents;
        bool m_isInEpoll;

        std::weak_ptr<void> m_tie;
        bool m_tied;

        EventLoop *m_loop;

        EventCallback m_readCallback;
        EventCallback m_writeCallback;
        EventCallback m_closeCallback;
        EventCallback m_errorCallback;

        void update();
        void handleEventWithGurad();
        static std::string eventsToString(int fd, int ev);
    };
}

#endif