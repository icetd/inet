#include "AsyncLogger.h"
#include "LogFile.h"
#include <unistd.h>

using namespace inet;

AsyncLogger::AsyncLogger(const std::string fileName, off_t rollSize, int flushInterval) : 
    m_flushInterval(flushInterval),
    m_is_running(false),
    m_basename(fileName),
    m_rollsize(rollSize),
    m_currentBuffer(std::make_unique<Buffer>()),
    m_nextBuffer(std::make_unique<Buffer>())
{
    m_currentBuffer->bzero();
    m_nextBuffer->bzero();
    m_buffers.reserve(16);
}

void AsyncLogger::append(const char *logline, int len)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_currentBuffer->available() > len) {
        m_currentBuffer->append(logline, len);
    } else {
        m_buffers.emplace_back(std::move(m_currentBuffer));
        m_currentBuffer.reset();

        if (m_nextBuffer) {
            m_currentBuffer = std::move(m_nextBuffer);
        } else {
            m_currentBuffer.reset(new Buffer);
        }
        m_currentBuffer->append(logline, len);
        m_cond.notify_one();
    }
}

void AsyncLogger::ThreadFunc()
{
    LogFile output(m_basename, m_rollsize);

    // 准备好后端备用的缓冲区1、2
    auto newBuffer1 = std::make_unique<Buffer>();
    auto newBuffer2 = std::make_unique<Buffer>();

    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    while (m_is_running)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_buffers.empty()) {
                m_cond.wait_for(lock, std::chrono::seconds(m_flushInterval));
            }

            m_buffers.push_back(std::move(m_currentBuffer));
            m_currentBuffer.reset();

            m_currentBuffer = std::move(newBuffer1);
            buffersToWrite.swap(m_buffers);

            if (!m_nextBuffer) {
                m_nextBuffer = std::move(newBuffer2);
            }
        } // lock out

        // 如果缓冲区过多，说明前端产生log的速度远大于后端消费的速度，这里只是简单的将它们丢弃
        if (buffersToWrite.size() > 25) {
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        // 将缓冲区内容写入文件
        for (size_t i = 0; i < buffersToWrite.size(); ++i) {
            output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }

        // 将过多的缓冲区丢弃
        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }

        // 恢复后端备用缓冲区
        if (!newBuffer1) {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            // 将缓冲区的数据指针归零
            newBuffer1->reset();
        }

        if (!newBuffer2) {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        // 丢弃无用的缓冲区
        buffersToWrite.clear();
        output.flush();
    }

    output.flush();
}