#ifndef INET_LOGGER_H
#define INET_LOGGER_H

#include "LogStream.h"
#include "TimeStamp.h"

void DefaultOutput(const char *msg, int len);
void AsyncOutput(const char *logline, int len);

namespace inet 
{
    class Logger
    {
    public:
        enum LogLevel {
            TRACE = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        Logger(const char *FileName, int line, LogLevel level, const char *funcName);
        Logger(const char *file, int line);
        Logger(const char *file, int line, LogLevel level);
        Logger(const char *file, int line, bool toAbort);
        ~Logger();

        LogStream &stream() { return m_impl.m_stream; }

        static LogLevel getGlobalLogLevel();
        static void setLogFileName(std::string filename) { m_log_file_basename = filename; }
        static std::string getLogFileName() { return m_log_file_basename; }
        
        using OutputFunc = void (*) (const char *msg, int len);
        using FlushFunc = void (*) ();

        static void setOutput(OutputFunc);
        static void setFlush(FlushFunc);
        static void setLogLevel(Logger::LogLevel level);
        
    private:
        class Impl 
        {
        public:
            using LogLevel = Logger::LogLevel;
            Impl(LogLevel level, const std::string &file, int line);
            void formatTime();
            void finish();

            TimeStamp m_time;
            LogStream m_stream;
            LogLevel m_level;
            int m_line;
            std::string m_filename;
        };

        Impl m_impl;

        static std::string m_log_file_basename;
    };

#define LOG_TRACE                                            \
    if (Logger::getGlobalLogLevel() <= Logger::LogLevel::TRACE) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::TRACE, __func__).stream()
#define LOG_DEBUG                                            \
    if (Logger::getGlobalLogLevel() <= Logger::LogLevel::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::DEBUG, __func__).stream()
#define LOG_INFO                                            \
    if (Logger::getGlobalLogLevel() <= Logger::LogLevel::INFO) \
    Logger(__FILE__, __LINE__).stream()

#define LOG_WARN Logger(__FILE__, __LINE__, Logger::LogLevel::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::LogLevel::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

} // namespace inet

#endif