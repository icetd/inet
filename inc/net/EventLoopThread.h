#ifndef INET_EVENTLOOPTHREAD_H
#define INET_EVENTLOOPTHREAD_H

#include "EventLoop.h"
#include "Thread.h"

namespace inet
{
    class EventLoopThread
    {
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;
        EventLoopThread();
        ~EventLoopThread();
        EventLoop *statloop();
    
    private:
        EventLoop *m_loop;
        Thread m_thread;
        std::mutex m_mutex;
        std::condition_variable m_cond;

        void threadFunc();
    };
} // end namespace

#endif