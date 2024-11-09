#include "InetAddress.h"
#include <string.h>

using namespace inet;

InetAddress::InetAddress()
{
    memset(&m_addr, 0, sizeof(m_addr));
}

InetAddress::InetAddress(unsigned short port, const char *ip)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);

    if (ip)
        inet_pton(AF_INET, ip, &m_addr.sin_addr.s_addr);
    else
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

std::string InetAddress::toIp() const
{
    char ip[64] = {0};
    inet_ntop(AF_INET, &m_addr.sin_addr.s_addr, ip, sizeof(ip));
    return ip;
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    sprintf(buf, "%s:%d", toIp().c_str(), toPort());
    return buf;
}

unsigned short InetAddress::toPort() const
{
    return ntohs(m_addr.sin_port);
}