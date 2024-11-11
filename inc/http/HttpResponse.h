#ifndef INET_HTTPRESPONSE_H
#define INET_HTTPRESPONSE_H

#include <string>
#include <unordered_map>
#include "Buffer.h"

namespace inet
{
    class HttpResponse 
    {
    public:
        enum class HttpStatusCode 
        {
            kUnknown = 0,
            k200OK = 200,
            k301MovedPermanently = 301,
            k400BadRequest = 400,
            k404NotFound = 404
        };

        explicit HttpResponse(bool close) :
            m_statusCode(HttpStatusCode::kUnknown),
            m_closeConnection(close)
        {}

        void setStatusCode(HttpStatusCode code) { m_statusCode = code; }
        void setStatusMessage(const std::string &message) { m_statusMessage = message; }
        void setCloseConnection(bool on) { m_closeConnection = on; }
        bool closeConnection() const { return m_closeConnection; }

        void setContentType(const std::string &contentType) { 
            addHeader("Content-Type", contentType);
        }
        void addHeader(const std::string &key, const std::string &value) {
            m_headers[key] = value;
        }

        void setBody(const std::string &body) { m_body = body; }

        void appendToBuffer(Buffer *output) const;
    
    private:
        std::unordered_map<std::string, std::string> m_headers;
        HttpStatusCode m_statusCode;
        std::string m_statusMessage;
        bool m_closeConnection;
        std::string m_body;
    };
}
#endif