#ifndef INET_TIMERQUEUE_H
#define INET_TIMERQUEUE_H

#include <set>
#include <vector>
#include <unordered_map>
#include <atomic>

#include "Channel.h"
#include "Timer.h"
#include "TimeStamp.h"
#include "EventLoop.h"
#include "Callbacks.h"

namespace inet 
{   
    class TimerQueue
    {
    public:
        explicit TimerQueue(EventLoop *loop);
        ~TimerQueue();

        int64_t addTimer(TimerCallback cb, TimeStamp when, double interval);
        void cancel(int64_t timerId);
    
    private:
        using Entry = std::pair<TimeStamp, Timer *>;
        using TimerList = std::set<Entry>;

        void addTimerInLoop(Timer *timer);
        void cancelInLoop(int64_t timerId);

        void handleRead();
        std::vector<Entry> getExpired(TimeStamp now); // get timeout timer
        void reset(const std::vector<Entry> &expired, TimeStamp now);
        bool insert(Timer *timer);

        EventLoop *m_loop;
        const int m_timerfd;
        Channel m_timerfd_channel;
        TimerList m_timers;
        std::unordered_map<int64_t, Timer*> m_activeTimers;
        std::unordered_map<int64_t, Timer*> m_cancelingTimers;
        std::atomic_bool m_callingExpiredTimers;
    };
} // end namespace

#endif