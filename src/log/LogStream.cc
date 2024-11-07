#include "LogStream.h"
#include <algorithm>
#include <string>
#include <string.h>
#include <stdint.h>

using namespace inet;

// 高效的整型数字转字符算法, by Matthew Wilson
template<typename T>
size_t convert(char buf[], T value)
{
    static const char digits[] = "9876543210123456789";
    static const char *zero = digits + 9;
    T i = value;
    char *p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while(i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

// 格式化数字类型为字符串并输出到缓冲区
template <typename T>
void LogStream::FromatInteger(T v)
{
    if (m_buffer.available() >= KMaxNumbericSize) {
        size_t len = convert(m_buffer.current(), v);
        m_buffer.appendComplete(len);
    }
}

LogStream &LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream &LogStream::operator<<(int v)
{
    FromatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(long v)
{
    FromatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(long long v)
{
    FromatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(float v)
{
    *this << static_cast<double>(v);
    return *this;
}

LogStream &LogStream::operator<<(double v)
{
    if (m_buffer.available() >= KMaxNumbericSize) {
        int len = snprintf(m_buffer.current(), KMaxNumbericSize, "%.12g", v);
        m_buffer.appendComplete(len);
    }

    return *this;
}

LogStream &LogStream::operator<<(char v)
{
    m_buffer.append(&v, 1);
    return *this;
}

LogStream &LogStream::operator<<(const char *v)
{
    if (v) {
        m_buffer.append(v, strlen(v));
    } else {
        m_buffer.append("(null)", 6);
    }
    return *this;
}

LogStream &LogStream::operator<<(const std::string &v)
{
    m_buffer.append(v.c_str(), v.size());
    return *this;
}