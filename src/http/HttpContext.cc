#include "HttpContext.h"

using namespace inet;

bool HttpContext::parseRequest(Buffer *buf)
{
    bool ok = true;
    bool hasMore = true;

    while (hasMore) {
        if (m_state == HttpRequestPaseState::kExpectRequestLine) { // 解析请求行
            // 查找出buf中第一次出现"\r\n"位置
            const char *crlf = buf->findCRLF();
            if (crlf) {
                // 若是找到"\r\n",说明至少有一行数据，可以进行解析
                // buf->peek()为数据开始部分
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) { // 解析成功
                    buf->retrieveUntil(crlf + 2); // buf->peek()向后移动2字节，到下一行
                    m_state = HttpRequestPaseState::kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (m_state == HttpRequestPaseState::kExpectHeaders) { // parse headers
            const char *crlf = buf->findCRLF(); // 找到"\r\n"位置
            if (crlf) {
                const char *colon = std::find(buf->peek(), crlf, ':'); // 定位分隔符
                if (colon != crlf) {
                    m_request.addHeader(buf->peek(), colon, crlf);
                } else {
                    m_state = HttpRequestPaseState::kExpectBody;
                }
                buf->retrieveUntil(crlf + 2); // 后移动2字节
            } else {
                hasMore = false;
            }
        } else if (m_state == HttpRequestPaseState::kExpectBody) { // parse body
            if (buf->readableBytes()) { // 表明还有数据，那就是请求体
                m_request.setQuery(buf->peek(), buf->beginWirte());
            }
            m_state = HttpRequestPaseState::kGotAll;
            hasMore = false;
        }
    }

    return ok;
}

bool HttpContext::parseRequest(Buffer *buf, TimeStamp receiveTime)  // parse request buffer
{
    bool ok = true;
    bool hasMore = true;

    while (hasMore) {
        if (m_state == HttpRequestPaseState::kExpectRequestLine) { // 解析请求行
            // 查找出buf中第一次出现"\r\n"位置
            const char *crlf = buf->findCRLF();
            if (crlf) {
                // 若是找到"\r\n",说明至少有一行数据，可以进行解析
                // buf->peek()为数据开始部分
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) { // 解析成功
                    m_request.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2); // buf->peek()向后移动2字节，到下一行
                    m_state = HttpRequestPaseState::kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (m_state == HttpRequestPaseState::kExpectHeaders) { // parse headers
            const char *crlf = buf->findCRLF(); // 找到"\r\n"位置
            if (crlf) {
                const char *colon = std::find(buf->peek(), crlf, ':'); // 定位分隔符
                if (colon != crlf) {
                    m_request.addHeader(buf->peek(), colon, crlf);
                } else {
                    m_state = HttpRequestPaseState::kExpectBody;
                }
                buf->retrieveUntil(crlf + 2); // 后移动2字节
            } else {
                hasMore = false;
            }
        } else if (m_state == HttpRequestPaseState::kExpectBody) { // parse body
            if (buf->readableBytes()) { // 表明还有数据，那就是请求体
                m_request.setQuery(buf->peek(), buf->beginWirte());
            }
            m_state = HttpRequestPaseState::kGotAll;
            hasMore = false;
        }
    }

    return ok;
}

bool HttpContext::processRequestLine(const char *begin, const char *end)
{
    // 请求行有固定格式Method URL Version CRLF，URL中可能带有请求参数。
    // 根据空格符进行分割成三段字符。URL可能带有请求参数，使用"?”分割解析
    bool succeed = true;

    const char *start = begin;
    const char *space = std::find(start, end, ' ');
    // 第一个空格前的字符串是请求方法 例如：GET
    // Method
    if (space != end && m_request.setMethod(start, space)) {
        start = space + 1;
    // URL    
        space = std::find(start, end, ' '); // 寻找第二个空格 url
        if (space != end) {
            const char *question = std::find(start, space, '?');
            if (question != space) { // 有"?"，分割成path和请求参数
                m_request.setPath(start, question);
                m_request.setQuery(question, space);
            } else {
                m_request.setPath(start, space); // 只有path
            }
    // VERSION
            start = space + 1;
            std::string version(start, end);
            if (version == "HTTP/1.0")
                m_request.setVersion(HttpRequest::Version::kHttp10);
            else if (version == "HTTP/1.1")
                m_request.setVersion(HttpRequest::Version::kHttp11);
            else
                succeed = false;
        }
    }
    return succeed;
}