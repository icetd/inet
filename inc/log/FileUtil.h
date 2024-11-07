#ifndef INET_FILEUTIL_H
#define INET_FILEUTIL_H

#include <string>

namespace inet
{
    class AppendFile
    {
    public:
        explicit AppendFile(const std::string &fileNmae);
        ~AppendFile();

        void append(const char *logline, size_t len);

        void flush();

        off_t writtenBytes() const { return m_writtenBytes; }

    private:
        FILE *m_fp;
        char m_buffer[1024 * 64];
        off_t m_writtenBytes;
    };
} //namespace inet

#endif