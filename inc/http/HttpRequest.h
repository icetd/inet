#ifndef INET_HTTPREQUEST_H
#define INET_HTTPREQUEST_H

#include <string>
#include <unordered_map>
#include "TimeStamp.h"

namespace inet
{   
    class HttpRequest
    {
    public:
        enum class Method
        {
            kInvalid = 0,
            kGet,
            kPost,
            kHead,
            kPut,
            kDelete
        };

        enum class Version
        {
            kUnknown = 0,
            kHttp10,
            kHttp11
        };

        HttpRequest() :
            m_method(Method::kInvalid) ,
            m_version(Version::kUnknown)
        {}

        void setVersion (Version v) { m_version = v; }
        Version getVersion() const { return m_version; }

        bool setMethod(const char *start, const char *end) {
            std::string m(start, end);
            if (m == "GET") {
                m_method = Method::kGet;
            } else if (m == "POST") {
                m_method == Method::kPost;
            } else if (m == "HEAD") {
                m_method == Method::kHead;
            } else if (m == "PUT") {
                m_method == Method::kPut;
            } else if (m == "DELETE") {
                m_method == Method::kDelete;
            }

            return m_method != Method::kInvalid;
        }

        Method getMethod () const { return m_method; }

        const char * methodString() const {
            const char *result = "UNKNOWN";
            
            switch (m_method) {
            case Method::kGet:
                result = "GET";
                break;

            case Method::kPost:
                result = "POST";
                break;
            
            case Method::kHead:
                result = "HEAD";
                break;

            case Method::kPut:
                result = "PUT";
                break;

            case Method::kDelete:
                result = "DELETE";
                break;
            
            default:
                break;
            }
            return result;
        }

        void setPath(const char *start, const char *end) {
            m_path.assign(start, end);
        }
        const std::string getPath() const { return m_path;}

        void setQuery(const char *start, const char *end) {
            m_query.assign(start, end);
        }
        const std::string getQuery() const { return m_query; }

        void setReceiveTime(TimeStamp t) { m_receiveTime = t; }
        TimeStamp getReceiveTime() const { return m_receiveTime; }

        void addHeader(const char *start, const char *colon, const char *end)
        {
            // isspace(int c)函数判断字符c是否为空白符
            // 当c为空白符时，返回非零值，否则返回零。（空白符指空格、水平制表、垂直制表、换页、回车和换行符。
            std::string filed(start, colon);
            ++colon;
            while (colon < end && std::isspace(*colon))
                ++colon;
            std::string value(colon, end);
            while(!value.empty() && std::isspace(value[value.size() -1]))
                value.resize(value.size() - 1);

            m_headers[filed] = value;
        }

        std::string getHeader(const std::string &filed) const {
            std::string result;
            auto it = m_headers.find(filed);
            if (it != m_headers.end()) {
                result = it->second;
            }
            return result;
        }

        void swap (HttpRequest &that)
        {
            std::swap(m_method, that.m_method);
            std::swap(m_version, that.m_version);
            m_path.swap(that.m_path);
            m_query.swap(that.m_query);
            m_headers.swap(that.m_headers);
        }

        const std::unordered_map<std::string, std::string> &getHeaders() const { return m_headers; }
        
    private:
        Method m_method;
        Version m_version;
        std::string m_path;  // request URL
        
        std::string m_query; // Body

        TimeStamp m_receiveTime;
        std::unordered_map<std::string, std::string> m_headers;
    };

}

#endif