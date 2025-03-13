#include "EventLoopThreadPool.h"

using namespace inet;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop) :
    m_baseLoop(baseloop),
    m_started(false),
    m_numThreads(0),
    m_next(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start()
{
    m_started = true;
    for (int i = 0; i < m_numThreads; i++) {
        auto t = new EventLoopThread;
        m_threads.push_back(std::unique_ptr<EventLoopThread>(t));

        // 底层创建线程，绑定一个新的EventLoop， 并返回该loop的地址
        m_loops.push_back(t->statloop());
    }
}

// simple
EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = m_baseLoop;
    if (!m_loops.empty()) {
        loop = m_loops[m_next];
        m_next = (m_next + 1) % m_numThreads;
    }
    return loop;
}
