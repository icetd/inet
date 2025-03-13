#ifndef INET_ASYNCLOGGER_H
#define INET_ASYNCLOGGER_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "LogStream.h"

namespace inet
{
    class AsyncLogger
    {
    public:
        using Buffer = FixedBuffer<KLargeBuffer>;
        using BufferPtr = std::unique_ptr<Buffer>;
        using BufferVector = std::vector<std::unique_ptr<Buffer>>;

        AsyncLogger(const std::string fileName, off_t rollSize, int flushInterval = 3);
        ~AsyncLogger()
        {
            if (m_is_running) {
                stop();
            }
        }

        void append(const char *logline, int len);

        void start()
        {
            m_is_running = true;
            m_thread = std::thread([this]() { ThreadFunc(); });
        }

        void stop()
        {
            m_is_running = false;
            m_cond.notify_one();
            m_thread.join();
        }

    private:
        void ThreadFunc();
        const int m_flushInterval;
        bool m_is_running;
        std::string m_basename;

        const off_t m_rollsize;
        std::thread m_thread;

        std::mutex m_mutex;
        std::condition_variable m_cond;
        BufferPtr m_currentBuffer; // current buffers
        BufferPtr m_nextBuffer;    // next buffer point
        BufferVector m_buffers;    // buffer vector for log thread
    };
} // namespace inet

#endif