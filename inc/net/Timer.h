#ifndef INET_TIMER_H
#define INET_TIMER_H

#include "Callbacks.h"
#include "TimeStamp.h"
#include <atomic>

namespace inet
{
    class Timer
    {
    public:
        Timer(TimerCallback cb, TimeStamp when, double interval) :
            m_callback(std::move(cb)),
            m_expiration(when),
            m_interval(interval),
            m_repeat(interval > 0.0),
            m_sequence(m_num_created++)
        {}

        void run() const { m_callback(); }
        TimeStamp expiration() const { return m_expiration; }
        int64_t sequence() const { return m_sequence; }
        bool repeat() const { return m_repeat; }
        void restart(TimeStamp now);
        static int64_t numCreated() { return m_num_created; }

    private:
        const TimerCallback m_callback;
        TimeStamp m_expiration;
        const double m_interval;  // 重复定时器执行事件的间隔时间，若是一次性定时器，该值为 0.0
        const bool m_repeat;      // 用于判断定时器是否是重复循环的
        const int64_t m_sequence; // 用来辨别定时器的唯一标识
        static std::atomic_int64_t m_num_created;
    };
} // namespace inet

#endif