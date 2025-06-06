#include "TimeStamp.h"
#include <chrono>

using namespace inet;

static_assert(sizeof(TimeStamp) == sizeof(int64_t),
              "TimeStamp should be same as int64_t");

std::string TimeStamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = m_micro_seconds_since_epoch / kMicroSecondsPerSecond;
    int64_t microseconds = m_micro_seconds_since_epoch % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%" PRId64, "%.06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string TimeStamp::toFormattedString(bool showMicroseconds) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(m_micro_seconds_since_epoch / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroseconds) {
        int microseconds = static_cast<int>(m_micro_seconds_since_epoch % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                 microseconds);
    } else {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

TimeStamp TimeStamp::now()
{
    auto counts = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    return TimeStamp(counts);
}