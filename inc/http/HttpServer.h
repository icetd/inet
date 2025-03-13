#ifndef INET_HTTPSERVER_H
#define INET_HTTPSERVER_H

#include <functional>
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

namespace inet
{
    class HttpServer
    {
    public:
        using HttpCallback = std::function<void(const HttpRequest &, HttpResponse *)>;
        HttpServer(EventLoop *loop, const InetAddress listenAddr);
        EventLoop *getLoop() const { return m_tcp_server.getLoop(); }
        void setHttpCallback(const HttpCallback &cb) { m_httpCallback = cb; }

        void start(int numThreads);

    private:
        TcpServer m_tcp_server;
        HttpCallback m_httpCallback;

        void onConnection(const TcpConnectionPtr &conn);
        void onMessage(const TcpConnectionPtr &conn, Buffer *buf);
        void onRequest(const TcpConnectionPtr &conn, const HttpRequest &req);
    };
} // namespace inet

#endif