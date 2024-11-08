#include "Timer.h"

using namespace inet;

std::atomic_int64_t Timer::m_num_created;

void Timer::restart(TimeStamp now)
{
    if (m_repeat) {
        m_expiration = addTime(now, m_interval);
    } else {
        m_expiration = TimeStamp::invalid();
    }
}