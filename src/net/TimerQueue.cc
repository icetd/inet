#include <sys/timerfd.h>
#include <assert.h>

#include "TimerQueue.h"
#include "Logger.h"

using namespace inet;

int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0){
		LOG_ERROR << "Failed in timerfd_create";
	}
	return timerfd;
}

struct timespec howMuchTimeFronNow(TimeStamp when)
{
	int64_t microseconds = when.microSecondsSinceEpoch() - TimeStamp::now().microSecondsSinceEpoch();
	if (microseconds < 100) {
		microseconds = 100;
	}

	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(microseconds / TimeStamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>((microseconds % TimeStamp::kMicroSecondsPerSecond) * 1000);
	return ts;
}

void resetTimerfd(int timerfd, TimeStamp expiration)
{
	struct itimerspec new_value;
	memset(&new_value, 0, sizeof(new_value));
	new_value.it_value = howMuchTimeFronNow(expiration);	//这个自定义函数是 获取expiration与当前时间的间隔
	int ret = ::timerfd_settime(timerfd, 0, &new_value, nullptr);	//启动定时器
	if (ret != 0) {
		LOG_ERROR << "timerfd_settime() error";
	}
}
void readTimerfd(int timerfd, TimeStamp now)
{
	uint64_t howmany;
	auto n = ::read(timerfd, &howmany, sizeof(howmany));
	//printf("TimerQueue::handleRead()  at %s\n", now.toString().c_str());
	if (n != sizeof(howmany)){
		LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
	}
}

TimerQueue::TimerQueue(EventLoop *loop) :
    m_loop(loop),
    m_timerfd(createTimerfd()),
    m_timerfd_channel(loop, m_timerfd)
{
    m_timerfd_channel.setReadCallback([this]() { handleRead(); });
    m_timerfd_channel.enableReading();
}

TimerQueue::~TimerQueue()
{
    m_timerfd_channel.disableAll();
    m_timerfd_channel.remove();
    close(m_timerfd);

    for (const auto &timer : m_timers) {
        delete timer.second;
    }
}

int64_t TimerQueue::addTimer(TimerCallback cb, TimeStamp when, double interval)
{
    auto timer = new Timer(std::move(cb), when, interval);
    m_loop->runInLoop([this, &timer]() { addTimerInLoop(timer); });
    return timer->sequence();
}

void TimerQueue::cancel(int64_t timerId)
{
    m_loop->runInLoop([this, timerId]() { cancelInLoop(timerId); });
}

void TimerQueue::addTimerInLoop(Timer *timer)
{  
    bool earliesChanged = insert(timer);
    if (earliesChanged) {
        resetTimerfd(m_timerfd, timer->expiration()); // reset timer
    }
}

void TimerQueue::cancelInLoop(int64_t timerId)
{
    auto it = m_activeTimers.find(timerId);
    if (it != m_activeTimers.end()) {
        m_timers.erase(Entry(it->second->expiration(), it->second));
        delete it->second;
        m_activeTimers.erase(it);
    } else if (m_callingExpiredTimers) {
        m_cancelingTimers.emplace(timerId, it->second);
    }
    assert(m_timers.size() == m_activeTimers.size()); //make sure size is same
}

void TimerQueue::handleRead()
{
	TimeStamp now(TimeStamp::now());
	readTimerfd(m_timerfd, now);	//只是简单的调用::read()函数回应。不回应的话，就会一直触发EPOLLIN的

	//获取已超时(大于now)的定时器 ，因为已超时的定时器可能不止一个
	auto expired = getExpired(now);

	m_callingExpiredTimers = true;
	m_cancelingTimers.clear();
	for (const auto& it : expired) {
		it.second->run();
	}
	m_callingExpiredTimers = false;

	reset(expired, now); //到期的定时器处理完后,需要进行重置最早的到期时间   
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now)
{
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	auto end = m_timers.lower_bound(sentry);	//找到第一个未到期的定时器

	std::vector<Entry> expired(m_timers.begin(), end);
	m_timers.erase(m_timers.begin(), end);	

	for (const auto& it : expired) {	
		m_activeTimers.erase(it.second->sequence());
	}
	assert(m_timers.size() == m_activeTimers.size());
	return expired;
}

void TimerQueue::reset(const std::vector<TimerQueue::Entry> &expired, TimeStamp now)
{
	for (const auto& it : expired) {
		//该定时器是重复的 && 没有在m_cancelingTimers容器中找到该定时器， 就再插入到容器中
		if (it.second->repeat() &&
			    m_cancelingTimers.find(it.second->sequence()) == m_cancelingTimers.end()) {
			it.second->restart(now);
			insert(it.second);
		} else {
			delete it.second;
		}
	}

	if (!m_timers.empty()) {
		TimeStamp nextExpire = m_timers.begin()->second->expiration();	//获取最早的超时时间
		if (nextExpire.valid()) {	//若时间是有效的，就以该时间进行重置最早的超时时间
			resetTimerfd(m_timerfd, nextExpire);
		}
	}
}

bool TimerQueue::insert(Timer *timer)
{
	assert(m_timers.size() == m_activeTimers.size());
	bool earliestChanged = false;
	auto when = timer->expiration();
	auto it = m_timers.begin();			
	if (it == m_timers.end() || when < it->first) {
		earliestChanged = true;	//这种情况，说明需要重置最早的到期时间
	}

	//在两个容器中都添加定时器
	m_timers.emplace(Entry(when, timer));
	m_activeTimers.emplace(timer->sequence(), timer);

	assert(m_timers.size() == m_activeTimers.size());
	return earliestChanged;
}