#include "EventLoopThread.h"

using namespace inet;

EventLoopThread::EventLoopThread() :
    m_loop(nullptr),
    m_thread([this]() { threadFunc(); })
{
}

EventLoopThread::~EventLoopThread()
{
    if (m_loop) {
        m_loop->quit();
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
}

EventLoop *EventLoopThread::statloop()
{
    m_thread.start();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_loop == nullptr) {
            m_cond.wait(lock);
        }
    }

    return m_loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond.notify_one();
    }

    loop.loop();

    std::lock_guard<std::mutex> lock(m_mutex);
    m_loop = nullptr;
}