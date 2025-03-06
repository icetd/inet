#include "WebSocketServer.h"
#include "HttpRequest.h"
#include "HttpContext.h"
#include "Buffer.h"
#include "Logger.h"
#include "WebSocketContext.h"
#include <memory>
#include <iostream>

using namespace inet;

WebSocketServer* WebSocketServer::instance_ = nullptr;

void defaultWebsocketCallback(const Buffer *buf, Buffer *sendBuf)
{
    // echo??return origin data
    sendBuf->append(buf->peek(), buf->readableBytes());
}

WebSocketServer::WebSocketServer(EventLoop *loop, const InetAddress &listenAddr) : 
    server_(loop, listenAddr),
    clientCloseCallback_(nullptr),
    websocketCallback_(nullptr)
{
    instance_ = this;
    server_.setConnectionCallback([this](const TcpConnectionPtr &conn) { onConnection(conn); });
    server_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf) { onMessage(conn, buf); });
    server_.setWriteCompleteCallback([this](const TcpConnectionPtr &conn) { onWriteComplete(conn); });
}

WebSocketServer::~WebSocketServer()
{
}

void WebSocketServer::start(int numThreads)
{
    server_.start(numThreads);
}

void WebSocketServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected()) {
        conn->setContext(std::make_shared<WebSocketContext>());
    }
}

void WebSocketServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf)
{
    WebSocketContext *context = reinterpret_cast<WebSocketContext*>(conn->getContext().get());
    if (!context)
    {
        printf("context kong...\n");
        LOG_ERROR << "context is bad\n";
        return;
    }

    if (context->getWebsocketSTATUS() == WebSocketContext::WebSocketSTATUS::kUnconnect) {
        HttpContext http;
        if (!http.parseRequest(buf)) {
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
        }

        if (http.gotAll()) {
            auto httpRequest = http.request();

            if (httpRequest.getHeader("Upgrade") != "websocket" ||
                (httpRequest.getHeader("Connection") != "keep-alive, Upgrade" && httpRequest.getHeader("Connection") != "Upgrade") ||
                httpRequest.getHeader("Sec-WebSocket-Version") != "13")
            {

                conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
                conn->shutdown();
                return; // is not wobsocket
            }

            Buffer handsharedbuf;
            context->handleShared(&handsharedbuf, http.request().getHeader("Sec-WebSocket-Key"));
            conn->send(&handsharedbuf);
            context->setwebsocketHandshared(); 
            if (clientConnectCallback_)
                clientConnectCallback_(conn);
        }
    } else {
        handleData(conn, context, buf);
    }
}

void WebSocketServer::onWriteComplete(const TcpConnectionPtr &conn)
{
    //std::cout << conn.use_count() << std::endl;
}

void WebSocketServer::handleData(const TcpConnectionPtr &conn, WebSocketContext *websocket, Buffer *buf)
{
    Buffer DataBuf;
    websocket->parseData(buf, &DataBuf);

    WebSocketPacket respondPacket;

    int opcode = websocket->getRequestOpcode();

    switch (opcode) {
    case WSOpcodeType::WSOpcode_Continue:
        // add your process code here
        respondPacket.set_opcode(WSOpcodeType::WSOpcode_Continue);
        break;
    case WSOpcodeType::WSOpcode_Text:
        // add your process code here
        respondPacket.set_opcode(WSOpcodeType::WSOpcode_Text);
        break;
    case WSOpcodeType::WSOpcode_Binary:
        // add your process code here
        respondPacket.set_opcode(WSOpcodeType::WSOpcode_Binary);
        break;
    case WSOpcodeType::WSOpcode_Close:
        
        // add your process code here
        if(clientCloseCallback_)
            clientCloseCallback_(conn);
        respondPacket.set_opcode(WSOpcodeType::WSOpcode_Close);
        break;
    case WSOpcodeType::WSOpcode_Ping:
        // add your process code here
        respondPacket.set_opcode(WSOpcodeType::WSOpcode_Pong);
        break;
    case WSOpcodeType::WSOpcode_Pong:
        // add your process code here
        return;
    default:
        LOG_INFO << "WebSocketEndpoint - recv an unknown opcode.\n";
        return;
    }
    if (opcode != WSOpcodeType::WSOpcode_Close && opcode != WSOpcode_Ping && opcode != WSOpcode_Pong) {
        if (websocketCallback_)
            websocketCallback_(&DataBuf, conn);
    } else {
        Buffer messageBuf;
        Buffer frameBuf;
        respondPacket.encodeFrame(&frameBuf, &messageBuf);

        if (!conn || !conn->connected() || conn.use_count() <= 1) {
            std::cerr << "WebSocket send failed: invalid or disconnected connection." << std::endl;
            return;
        }
        conn->send(&frameBuf);
    }

    websocket->reset();
}

void WebSocketServer::send(const char *data, size_t len, const WSCodeType type, const std::weak_ptr<TcpConnection> &conn)
{
    WebSocketContext *context = reinterpret_cast<WebSocketContext*>(conn.lock()->getContext().get());
    if (!context)
    {
        printf("context kong...\n");
        LOG_ERROR << "context is bad\n";
        return;
    }

    WebSocketPacket packet;
    packet.set_opcode(type);
    Buffer messageBuf;
    messageBuf.append(data, len);

    Buffer frameBuf;
    packet.encodeFrame(&frameBuf, &messageBuf);

    try {
        conn.lock()->send(&frameBuf);
        context->reset();
    } catch (const std::exception &e) {
        std::cerr << "WebSocket send error: " << e.what() << std::endl;
    }
}