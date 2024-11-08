#ifndef INET_EVENTLOOPTHREADPOOL_H
#define INET_EVENTLOOPTHREADPOOL_H

#include "EventLoopThread.h"

namespace inet
{
    class EventLoopThreadPool
    {
    public:
        EventLoopThreadPool(EventLoop *baseloop);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads) { m_numThreads = numThreads; }
        void start();

        EventLoop *getNextLoop();

        bool started() const { return m_started; }

    private:
        EventLoop *m_baseLoop; // 与Acceptor所属EventLoop相同
        bool m_started;
        int m_numThreads;
        int m_next;                                              // 新连接到来，所选择的EventLoopThread下标
        std::vector<std::unique_ptr<EventLoopThread>> m_threads; // IO线程列表
        std::vector<EventLoop *> m_loops;                        // EventLoop列表, 指向的是EventLoopThread线程函数创建的EventLoop对象
    };
}

#endif