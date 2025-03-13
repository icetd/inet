#include "FileUtil.h"
#include "Logger.h"

using namespace inet;
AppendFile::AppendFile(const std::string &fileNmae) :
    m_fp(fopen(fileNmae.c_str(), "ae")),
    m_writtenBytes(0)
{
    if (m_fp == nullptr) {
        LOG_ERROR << "log file open failed: error = " << errno << " reson = " << strerror(errno);
    } else {
        setbuffer(m_fp, m_buffer, sizeof(m_buffer));
    }
}

AppendFile::~AppendFile()
{
    if (m_fp)
        fclose(m_fp);
}

void AppendFile::append(const char *logline, size_t len)
{
    size_t written = 0;
    while (written != len) {
        auto remain = len - written;
        size_t n = fwrite_unlocked(logline + written, 1, remain, m_fp);
        if (n != remain) {
            int err = ferror(m_fp);
            if (err) {
                fprintf(stderr, "AppendFile::append failed %s\n", strerror(err));
                break;
            }
        }
        written += n;
    }
    m_writtenBytes += written;
}

void AppendFile::flush()
{
    fflush(m_fp);
}
