#ifndef INET_EVENTLOOP_H
#define INET_EVENTLOOP_H

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include "Channel.h"
#include "Epoll.h"
#include "CurrentThread.h"

namespace inet
{   
    class TimerQueue;
    class Channel;
    class Epoll;
    
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
    
    private:
        pid_t m_threadId;
        std::atomic_bool m_quit;
        std::atomic_bool m_callingPendingFunctors;
        
        std::unique_ptr<Epoll> m_ep;
        channelList m_activeChannels;

        int m_wakeupFd;
        std::unique_ptr<Channel> m_wakeupChannel;
        std::unique_ptr<TimerQueue> m_timerqueue;
        std::unique_ptr<Functor> m_pendingFunctors;
        std::mutex m_mutex;
    };
}

#endif