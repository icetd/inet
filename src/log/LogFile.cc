#include "LogFile.h"
#include "Util.h"
#include <string.h>

using namespace inet;

LogFile::LogFile(const std::string &fileName, off_t rollSize, int flushInterval_seconds, int flushEveryN) :
    m_basename(fileName),
    m_rollsize(rollSize),
    m_flush_interval_seconds(flushInterval_seconds),
    m_fluse_everyN(flushEveryN),
    m_count(0),
    m_start_of_period(0),
    m_last_roll(0),
    m_last_flush(0)
{
    rollFile();
}

void LogFile::append(const char *logline, int len)
{
    m_file->append(logline, len);
    if (m_file->writtenBytes() > m_rollsize) {
        rollFile();
    } else {
        m_count++;
        if (m_count > m_fluse_everyN) {
            time_t now = ::time(nullptr);
            time_t this_period = now / kRollPerSeconds * kRollPerSeconds;
            if (this_period != m_start_of_period) {
                rollFile();
            } else if (now - m_last_flush > m_flush_interval_seconds) {
                m_last_flush = now;
                m_file->flush();
            }
        }
    }
}

void LogFile::flush()
{
    m_file->flush();
}

bool LogFile::rollFile()
{
    time_t now = time(nullptr);
    if (now > m_last_roll) {
        std::string filename = getLogFileName(m_basename, &now);
        m_last_roll = now;
        m_last_flush = now;
        m_start_of_period = now / kRollPerSeconds * kRollPerSeconds;
        m_file.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string &basename, time_t *now)
{
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm_result;
    memset(&tm_result, 0, sizeof(tm_result));
    localtime_r(now, &tm_result);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm_result);
    filename += timebuf;

    filename += ProcessInfo::hostname();

    char pidbuf[32];
    snprintf(pidbuf, sizeof(pidbuf), ".%d.log", ProcessInfo::pid());
    filename += pidbuf;

    return filename;
}
