#ifndef INET_EVENTLOOP_H
#define INET_EVENTLOOP_H

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <functional>

#include "CurrentThread.h"
#include "Callbacks.h"
#include "Epoll.h"
#include "Channel.h"
#include "Timer.h"
#include "TimeStamp.h"

namespace inet
{   
    class TimerQueue;
    class EventLoop
    {
    public:
        using Functor = std::function<void()>;
        using channelList = std::vector<Channel*>;

        EventLoop();
        ~EventLoop();

        void loop();
        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);

        bool isInLoopThread() const { return m_threadId == CurrentThread::tid(); }

        void runInLoop(Functor cb);
        void queueInLoop(Functor cb);

        void wakeup();
        pid_t getThreadId() const { return m_threadId; }

        void quit();
        
        void assertInLoopThread();

        int64_t runAt(TimeStamp time, TimerCallback cb);
        int64_t runAfter(double delay_seconds, TimerCallback cb);
        int64_t runEvery(double interval_seconds, TimerCallback cb);
        void cancel(int64_t timerId);

    private:
        pid_t m_threadId;
        std::atomic_bool m_quit;
        std::atomic_bool m_callingPendingFunctors;
        
        std::unique_ptr<Epoll> m_ep;
        channelList m_activeChannels;

        int m_wakeupFd;
        std::unique_ptr<Channel> m_wakeupChannel;
        std::unique_ptr<TimerQueue> m_timerQueue;
        std::vector<Functor> m_pendingFunctors;
        std::mutex m_mutex;

        void doPendingFunctors();  //do task callback
        void handleRead(); // for wake ip
    };  
}

#endif