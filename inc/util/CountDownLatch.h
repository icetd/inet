#ifndef INET_COUNTDOWNLATCH_H
#define INET_COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>

namespace inet
{
    class CountDownLatch
    {
    public:
        explicit CountDownLatch(int count);

        void wait();

        void countDown();

        int getCount();

    private:
        std::mutex m_mutex;
        std::condition_variable m_cond;
        int m_count;
    };
}

#endif