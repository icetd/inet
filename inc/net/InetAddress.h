#ifndef INET_INETADDRESS_H
#define INET_INETADDRESS_H

#include <string>
#include "Platform.h"

namespace inet
{
    class InetAddress
    {
    public:
        InetAddress();
        InetAddress(unsigned short port, const char *ip = nullptr);
        explicit InetAddress(const struct sockaddr_in &address) :
            m_addr(address) {}

        const struct sockaddr_in *getSockAddr() const { return &m_addr; }

        void setAddr(const struct sockaddr_in &address) { m_addr = address; }
        std::string toIp() const;
        std::string toIpPort() const;
        unsigned short toPort() const;

    private:
        struct sockaddr_in m_addr;
    };

} // namespace inet

#endif