#ifndef INET_HTTPCONTEXT_H
#define INET_HTTPCONTEXT_H

#include "Buffer.h"
#include "HttpRequest.h"

namespace inet
{
    class HttpContext
    {
    public:
        enum class HttpRequestPaseState
        {
            kExpectRequestLine,
            kExpectHeaders,
            kExpectBody,
            kGotAll
        };

        HttpContext() :
            m_state(HttpRequestPaseState::kExpectRequestLine)
        {}

        bool parseRequest(Buffer *buf);
        bool parseRequest(Buffer *buf, TimeStamp receiveTime); // parse request buffer

        bool gotAll() const { return m_state == HttpRequestPaseState::kGotAll; }
        
        void reset() {
            m_state = HttpRequestPaseState::kExpectRequestLine;
            HttpRequest dumy;
            m_request.swap(dumy);
        }

        const HttpRequest &request() const {
            return m_request;
        }

        HttpRequest request() {
            return m_request;
        }

    private:
        bool processRequestLine(const char *begin, const char *end);
        
        HttpRequestPaseState m_state;
        HttpRequest m_request;
    };
}



#endif