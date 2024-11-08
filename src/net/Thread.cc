#include "Thread.h"
#include "CurrentThread.h"

using namespace inet;

Thread::Thread(ThreadFunc func) :
    m_started(false),
    m_joined(false),
    m_func(std::move(func)),
    m_latch(1)
{
}

Thread::~Thread()
{
	if (m_started && !m_joined)
		m_thread.detach();
}

void Thread::start()
{
	m_started = true;

	m_thread = std::move(std::thread([this]() {
		m_tid = CurrentThread::tid();
		m_latch.countDown();
		m_func();
	    }));

	m_latch.wait();
}

void Thread::join()
{
	if (m_thread.joinable()) {
		m_thread.join();
		m_joined = true;
	}
}