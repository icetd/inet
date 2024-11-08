#include "EventLoop.h"
#include "TimerQueue.h"
#include "Logger.h"
#include <iostream>

using namespace inet;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        std::cout << "eventfd error: " << errno << std::endl;
    }
    return evtfd;
}

class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe initObj;

EventLoop::EventLoop() :
    m_threadId(CurrentThread::tid()),
    m_quit(false),
    m_callingPendingFunctors(false),
    m_ep(std::make_unique<Epoll>()),
    m_wakeupFd(createEventfd()),
    m_wakeupChannel(std::make_unique<Channel>(this, m_wakeupFd)),
    m_timerQueue(std::make_unique<TimerQueue>(this))
{
    m_wakeupChannel->setReadCallback([this]() { handleRead(); });
    m_wakeupChannel->enableReading();
}

EventLoop::~EventLoop()
{
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    ::close(m_wakeupFd);
}

// observer mode
void EventLoop::loop()
{
    m_quit = false;
    while (!m_quit) {
        m_activeChannels.clear();
        m_ep->epollWait(m_activeChannels, 10000);
        for (auto &active : m_activeChannels) {
            active->handleEvent();
        }

        // do Callbacks;
        doPendingFunctors();
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    m_ep->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    m_ep->destroy(channel);
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_pendingFunctors.push_back(std::move(cb));
	}

	if (!isInLoopThread() || m_callingPendingFunctors) {
		wakeup();
	}
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = write(m_wakeupFd, &one, sizeof(one));
	if (n != sizeof(one)){
		LOG_DEBUG<< "EventLoop wakeup write "<< n <<" bytes instead of 8";
	}
}

void EventLoop::quit()
{
    m_quit = true;
}

void EventLoop::assertInLoopThread()
{
    if (!isInLoopThread()) {
        LOG_ERROR << "not in this loopThread";
    } 
}

//在给定的时间执行定时器
int64_t EventLoop::runAt(TimeStamp time, TimerCallback cb)
{
    return m_timerQueue->addTimer(std::move(cb), time, 0.0);
}

//在给定的时间间隔后执行定时器
int64_t EventLoop::runAfter(double delay_seconds, TimerCallback cb)
{
    TimeStamp time(addTime(TimeStamp::now(), delay_seconds));
	return runAt(time, std::move(cb));
}

//每个一个时间间隔就执行一次定时器
int64_t EventLoop::runEvery(double interval_seconds, TimerCallback cb)
{
	TimeStamp time(addTime(TimeStamp::now(), interval_seconds));
	return m_timerQueue->addTimer(std::move(cb), time, interval_seconds);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;
    
    // 把functors转移到局部的functors，这样在执行回调时不用加锁。不影响mainloop注册回调
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }

    for (const auto &functor : functors) {
        functor();
    }

    m_callingPendingFunctors = false;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
	auto n = ::read(m_wakeupFd, &one, sizeof(one));
	if (n != sizeof(one)){
		LOG_INFO << "EventLoop::handleRead() reads " << n << " bytes";
	}
}