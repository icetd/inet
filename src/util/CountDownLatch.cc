#include"CountDownLatch.h"

using namespace inet;


CountDownLatch::CountDownLatch(int count)
	:m_mutex()
	,m_cond()
	,m_count(count)
{
}

void CountDownLatch::wait()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	while (m_count > 0) {
		m_cond.wait(lock);
	}
}

void CountDownLatch::countDown()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	--m_count;
	if (m_count == 0) {
		m_cond.notify_all();
	}
}

int CountDownLatch::getCount()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_count;
}