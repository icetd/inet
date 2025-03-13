#include "TcpConnection.h"
#include "Util.h"
#include "Logger.h"

#include <thread>
#include <iostream>

using namespace inet;

TcpConnection::TcpConnection(EventLoop *loop, int sockfd, const InetAddress &loaclAddr, const InetAddress &peerAddr) :
    m_loop(loop),
    m_state(StateE::kConnecting),
    m_socket(std::make_unique<Socket>(sockfd)),
    m_channel(std::make_unique<Channel>(loop, sockfd)),
    m_localAddr(loaclAddr),
    m_peerAddr(peerAddr)
{
    m_channel->setReadCallback([this]() { handleRead(); });
    m_channel->setWriteCallback([this]() { handleWrite(); });
    m_channel->setCloseCallback([this]() { handleClose(); });
    m_channel->setErrorCallback([this]() { handleError(); });
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor at  fd= " << m_channel->getFd() << " state= " << static_cast<int>(m_state);
}

void TcpConnection::send(Buffer *message)
{
    send(message->peek(), message->readableBytes());
    message->retrieveAll();
}

void TcpConnection::send(const char *message, size_t len)
{
    if (m_state == StateE::kConnected) {
        if (m_loop->isInLoopThread()) {
            sendInLoop(message, len);
        } else {
            m_loop->runInLoop([this, message, len]() { sendInLoop(message, len); });
        }
    }
}

void TcpConnection::send(const std::string &message)
{
    send(message.data(), message.size());
}

void TcpConnection::shutdown()
{
    if (m_state == StateE::kConnected) {
        setState(StateE::kDisconnecting);
        m_loop->runInLoop([this]() { shutdownInLoop(); });
    }
}
void TcpConnection::shutdownInLoop()
{
    if (!m_channel->isWrite()) {
        sockets::shutdownWrite(fd()); // 关闭写端 ,能触发EPOLLHUP,也会触发EPOLLIN
    }
}
void TcpConnection::forceClose()
{
    if (m_state == StateE::kConnected || m_state == StateE::kDisconnecting) {
        setState(StateE::kDisconnecting);
        // 使用queueInLoop函数，就一定是放在任务队列中，即是在EventLoop::doPendingFunctors()中执行forceCloseInLoop()，需要使用shared_from_this()
        m_loop->queueInLoop([this]() { shared_from_this()->forceCloseInLoop(); });
    }
}

void TcpConnection::forceCloseInLoop()
{
    m_loop->assertInLoopThread();
    if (m_state == StateE::kConnected || m_state == StateE::kDisconnecting) {
        setState(StateE::kDisconnecting);
        handleClose();
    }
}
void TcpConnection::connectEstablished()
{
    assert(m_state == StateE::kConnecting);
    setState(StateE::kConnected);
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
    m_connectionCallback(shared_from_this()); // 调用用户设置的连接成功或断开的回调函数
}

// 连接销毁(关闭连接的最后一步)
void TcpConnection::connectDestroyed()
{
    if (m_state == StateE::kConnected) { // 一般不会进入这个if
        setState(StateE::kDisconnected);
        m_channel->disableAll();

        m_connectionCallback(shared_from_this()); // 调用用户设置的连接成功或断开的回调函数
    }
    m_channel->remove();
    // printf("TcpConnection::connectDestroyed()..\n");
}

void TcpConnection::handleRead()
{
    int savedErrno = 0;
    auto n = m_inputBuffer.readFd(fd(), &savedErrno);
    if (n > 0) {
        m_messageCallback(shared_from_this(), &m_inputBuffer);
        m_inputBuffer.retrieve(m_inputBuffer.readableBytes()); // m_messageCallback中处理好读取的数据后，更新readerIndex位置
    } else if (n == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (!m_channel->isWrite()) {
        LOG_INFO << "TcpConnection fd = " << m_channel->getFd() << "  is down, no more writing";
        return;
    }

    auto n = ::write(fd(), m_outputBuffer.peek(), m_outputBuffer.readableBytes());
    if (n > 0) {
        // 更新readerIndex
        m_outputBuffer.retrieve(n);
        if (m_outputBuffer.readableBytes() == 0) { // 表明要发送的数据已全部发送完毕，所以取消写事件
            m_channel->disableWriting();
        } else {
            LOG_INFO << "read to write more data";
        }
    } else {
        LOG_ERROR << "handleWrite error";
    }
}

void TcpConnection::handleClose()
{
    assert(m_state == StateE::kConnected || m_state == StateE::kDisconnecting);
    setState(StateE::kDisconnected);
    m_channel->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    m_closeCallback(guardThis); // m_closeCallback就是TcpServer::removeConnection()函数
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(m_channel->getFd());
    LOG_DEBUG << "TcpConnection::handleError() error=" << err;
}

void TcpConnection::sendInLoop(const char *message, size_t len)
{
    if (m_state == StateE::kDisconnected) {
        LOG_DEBUG << "disconnected, give up writing";
        return;
    }

    bool faultError = false;
    ssize_t nwrote = 0;
    size_t reamining = len;
    if (!m_channel->isWrite() && m_outputBuffer.readableBytes() == 0) {
        nwrote = ::write(fd(), message, len);
        if (nwrote >= 0) {
            reamining = len - nwrote;
            if (reamining == 0) {
                // 表示数据已完全发送出去，通知用户写已完成
                if (m_writeCompleteCallback) {
                    m_writeCompleteCallback(shared_from_this());
                }
            }
        } else { // nwrote<0
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && reamining > 0) {
        m_outputBuffer.append(static_cast<const char *>(message) + nwrote, reamining);
        if (!m_channel->isWrite()) {
            m_channel->enableWriting();
        }
    }
}