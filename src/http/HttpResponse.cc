#include "HttpResponse.h"
#include <iostream>
using namespace inet;

void HttpResponse::appendToBuffer(Buffer *output) const
{
    char buf[32];
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", (int)m_statusCode);
    output->append(buf);
    output->append(m_statusMessage);
    output->append("\r\n");

    if (m_closeConnection) {
        output->append("Connection: close\r\n");
    } else {
        output->append("Access-Control-Allow-Origin: *\r\n");                            // 允许所有域名访问
        output->append("Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n");      // 允许的方法
        output->append("Access-Control-Allow-Headers: Content-Type, Authorization\r\n"); // 允许的请求头
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", m_body.size());
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n");
    }

    for (const auto &header : m_headers) {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    output->append(m_body);
}