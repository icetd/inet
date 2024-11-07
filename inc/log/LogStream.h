#ifndef INET_LOGSTREAM_H
#define INET_LOGSTREAM_H

#include <string>
#include <string.h>

namespace inet
{
    const int KSmallBuffer = 1024;
    const int KLargeBuffer = 1024 * 1000;

    template<int SIZE>
    class FixedBuffer
    {
    public:
        FixedBuffer() : m_cur(m_data) {}
        ~FixedBuffer() {}
        
        void append(const char *buf, size_t len)
        {
            if (available() > static_cast<int>(len)) {
                memcpy(m_cur, buf, len);
                appendComplete(len);
            }
        }

        void appendComplete(size_t len) { m_cur += len; }
        void reset() { m_cur = m_data; }
        void bzero() { memset(m_data, 0, sizeof(m_data)); }
        const char *data() const { return m_data; }
        int length() const { return static_cast<int>(m_cur - m_data); }
        char *current() { return m_cur; }
        int available() const { return static_cast<int>(end() - m_cur); }
    private:
        char m_data[SIZE];
        char *m_cur;
        
        const char *end() const { return m_data + sizeof(m_data); }
    };

    class LogStream
    {
    public:
        using Buffer = FixedBuffer<KSmallBuffer>;

        LogStream & operator << (bool v) {
            m_buffer.append(v ? "1" : "0", 1);
            return *this;
        }

        LogStream &operator<<(short);
        LogStream &operator<<(int);
        LogStream &operator<<(long);
        LogStream &operator<<(long long);
        LogStream &operator<<(float);
        LogStream &operator<<(double);
        LogStream &operator<<(char);
        LogStream &operator<<(const char *);
        LogStream &operator<<(const std::string &);

        void append(const char *data, int len) { m_buffer.append(data, len); }
        const Buffer &buffer() const { return m_buffer; }

    private:
        template<class T>
        void FromatInteger(T);

        Buffer m_buffer;

        static const int KMaxNumbericSize = 48;
    };
}

#endif