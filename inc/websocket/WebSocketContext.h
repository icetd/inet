#ifndef INET_WEBSOCKET_CONTEXT
#define INET_WEBSOCKET_CONTEXT

#include "Buffer.h"
#include "WebSocketPacket.h"

namespace inet
{
    class WebSocketContext
    {
    public:
        enum class WebSocketSTATUS
        {
            kUnconnect,
            kHandsharked
        };

        WebSocketContext();
        ~WebSocketContext();

        void handleShared(Buffer *buf, const std::string &server_key);

        void parseData(Buffer *buf, Buffer *output);
        void reset() { m_requestPacket.reset(); }

        void setwebsocketHandshared() { m_websocketStatus = WebSocketSTATUS::kHandsharked; }
        WebSocketSTATUS getWebsocketSTATUS() const { return m_websocketStatus; }

        uint8_t getRequestOpcode() const { return m_requestPacket.opcode(); }

    private:
        WebSocketSTATUS m_websocketStatus;

        WebSocketPacket m_requestPacket;
    };

} // namespace inet

#endif