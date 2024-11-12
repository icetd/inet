#include "HttpResponse.h"

using namespace inet;

void HttpResponse::appendToBuffer(Buffer *output) const
{   
    // status line
    std::string buf = "HTTP/1.1 " + std::to_string(static_cast<int>(m_statusCode));
    output->append(buf);
    output->append(m_statusMessage);
    output->append("\r\n");
    
    // headres line
    if(m_closeConnection) {
        output->append("Connection: close\r\n");
    } else {
        output->append("Access-Control-Allow-Origin: *\r\n");                            // 允许所有域名访问
        output->append("Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n");      // 允许的方法
        output->append("Access-Control-Allow-Headers: Content-Type, Authorization\r\n"); // 允许的请求头
        output->append("Connection: Keep-Alive\r\n");
        buf = "Content-Length:" + std::to_string(m_body.size()) + "\r\n";
        output->append(buf);
    }

    for (const auto &header : m_headers) {
        buf = header.first + ": " + header.second + "\r\n";
        output->append(buf);
    }

    // black line
    output->append("\r\n");
    // body
    output->append(m_body);
}