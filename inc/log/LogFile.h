#ifndef INET_LOGFILE_H
#define INET_LOGFILE_H

#include <string>
#include <memory>
#include "FileUtil.h"

namespace inet
{
    class LogFile
    {
    public:
        LogFile(const std::string &fileName, off_t rollSize, int flushInterval_seconds = 3, int flushEveryN = 1024);
        void append(const char *logline, int len);
        void flush();
        bool rollFile();

    private:
        const std::string m_basename;
        const off_t m_rollsize;
        const int m_flush_interval_seconds;
        const int m_fluse_everyN;
        int m_count;

        std::unique_ptr<AppendFile> m_file;

        time_t m_start_of_period; // log start time
        time_t m_last_roll;       // last log roll time
        time_t m_last_flush;      // last flush log time

        const static int kRollPerSeconds = 60 * 60 * 24; // one day seconds

        std::string getLogFileName(const std::string &basename, time_t *now);
    };

} // namespace inet

#endif