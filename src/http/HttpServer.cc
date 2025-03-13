#include <memory>
#include "HttpServer.h"
#include "HttpContext.h"
#include "Logger.h"

using namespace inet;

void defaultHttpCallback(const HttpRequest &req, HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop, const InetAddress listenAddr) :
    m_tcp_server(loop, listenAddr),
    m_httpCallback([](const HttpRequest &req, HttpResponse *resp) { defaultHttpCallback(req, resp); })
{
    m_tcp_server.setConnectionCallback([this](const TcpConnectionPtr &conn) { onConnection(conn); });
    m_tcp_server.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf) { onMessage(conn, buf); });
}

void HttpServer::start(int numThreads)
{
    m_tcp_server.start(numThreads);
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected()) {
        conn->setContext(std::make_shared<HttpContext>());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf)
{
    HttpContext *context = reinterpret_cast<HttpContext *>(conn->getContext().get());

    if (!context) {
        LOG_WARN << "context is bad";
        return;
    }

    if (!context->parseRequest(buf)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll()) {
        onRequest(conn, context->request());
        context->reset(); // reset context unbind HttpContext && TcpConnection
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req)
{
    const std::string &connection = req.getHeader("Connection");
    bool close = (connection == "close") || (req.getVersion() == HttpRequest::Version::kHttp10 && connection != "Keep-Alive");

    HttpResponse response(close);

    m_httpCallback(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection()) {
        conn->shutdown();
    }
}