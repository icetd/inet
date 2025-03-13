#ifndef INET_THREAD_H
#define INET_THREAD_H

#include <functional>
#include <thread>

#include "CountDownLatch.h"

namespace inet
{
    class Thread
    {
    public:
        using ThreadFunc = std::function<void()>;
        explicit Thread(ThreadFunc);
        ~Thread();

        void start();
        void join();
        bool started() { return m_started; }
        pid_t tid() const { return m_tid; }

        bool joinable() const { return m_thread.joinable(); }

    private:
        bool m_started;
        bool m_joined;

        std::thread m_thread;
        pid_t m_tid;
        ThreadFunc m_func;

        CountDownLatch m_latch;
    };
} // namespace inet

#endif