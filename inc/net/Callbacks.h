#ifndef INET_CALLBACKS_H
#define INET_CALLNACKS_H

#include <functional>
#include <memory>

namespace inet
{
    class Buffer;
    class TcpConnection;

    using TimerCallback = std::function<void()>;
    
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
    using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *)>;
}
#endif