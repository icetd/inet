#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <functional>
#include <unordered_map>
#include <memory>
#include "TcpServer.h"
#include "WebSocketPacket.h"
#include "WebSocketContext.h"

namespace inet
{

    class WebSocketServer
    {
    public:
        using WebsocketCallback = std::function<void(const Buffer*, const TcpConnectionPtr &conn)>;
        using ClientCloseCallback = std::function<void(const TcpConnectionPtr &conn)>;
        using ClientConnectCallback = std::function<void(const TcpConnectionPtr &conn)>;

        static WebSocketServer* getInstance() {
            if (instance_)
                return instance_;
            else 
                return nullptr;
        }

        WebSocketServer(EventLoop *loop, const InetAddress &listenAddr);
        ~WebSocketServer();

        EventLoop *getLoop() const { return server_.getLoop(); }

        void setHttpCallback(const WebsocketCallback &cb) { websocketCallback_ = cb; }
        void setClientConnectCallback(const ClientConnectCallback &cb) { clientConnectCallback_ = cb; }
        void setClientCloseCallback(const ClientCloseCallback &cb) { clientCloseCallback_ = cb; }
        void start(int numThreads);
        void send(const char *data, size_t len, const WSCodeType type, const std::weak_ptr<TcpConnection> &conn);

    private:
        void onConnection(const TcpConnectionPtr &conn);
        void onMessage(const TcpConnectionPtr &conn, Buffer *buf);
        void onWriteComplete(const TcpConnectionPtr &conn);
        void handleData(const TcpConnectionPtr &conn, WebSocketContext *websocket, Buffer *buf);

        static WebSocketServer* instance_;
        TcpServer server_;
        WebsocketCallback websocketCallback_;
        ClientCloseCallback clientCloseCallback_;
        ClientConnectCallback clientConnectCallback_;
    };

}

#endif