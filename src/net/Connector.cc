#include "Connector.h"
#include "Logger.h"
#include "assert.h"
#include "Util.h"
#include <iostream>

using namespace inet;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr) :
    m_loop(loop),
    m_serverAddr(serverAddr),
    m_connect(false),
    m_state(States::kDisconnected),
    m_retryDelayMs(kInitRetryDelayMs)
{
    std::cout << m_serverAddr.toIpPort();
}

Connector::~Connector()
{
    assert(!m_channel);
}

void Connector::start()
{
    m_connect = true;
    m_loop->runInLoop([this]() { startInLoop(); });
}

void Connector::restart()
{
    m_loop->assertInLoopThread();
    setState(States::kDisconnected);
    m_retryDelayMs = kInitRetryDelayMs;
    m_connect = true;
    startInLoop();
}

void Connector::stop()
{
    m_connect = false;
    m_loop->queueInLoop([this]() { stopInLoop(); });
}

void Connector::startInLoop()
{
    m_loop->assertInLoopThread();
    assert(m_state == States::kDisconnected);
    if (m_connect) {
        connect();
    } else {
    }
}

void Connector::stopInLoop()
{
    m_loop->assertInLoopThread();
    if (m_state == States::kConnecting) {
        setState(States::kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = sockets::createNonblockingOrDie();
    int ret = sockets::connect(sockfd, m_serverAddr.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;

    switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
        sockets::close(sockfd);
        break;

    default:
        LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
        sockets::close(sockfd);
        break;
    }
}

void Connector::connecting(int sockfd)
{
    setState(States::kConnecting);
    assert(!m_channel);
    m_channel.reset(new Channel(m_loop, sockfd));
    m_channel->setWriteCallback([this]() { handleWrite(); });
    m_channel->setErrorCallback([this]() { handleError(); });

    m_channel->enableWriting();
}

void Connector::handleWrite()
{
    LOG_DEBUG << "Connector::handleWrite";
    if (m_state == States::kConnecting) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if (err) {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = " << err << " " << strerror(err);
            retry(sockfd);
        } else if (sockets::isSelfConnect(sockfd)) {
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        } else {
            setState(States::kConnected);
            if (m_connect) {
                m_newConnectionCallback(sockfd);
            } else {
                sockets::close(sockfd);
            }
        }
    } else {
        assert(m_state == States::kDisconnected);
    }
}

void Connector::handleError()
{
    LOG_ERROR << "Connector::handleError";
    if (m_state == States::kConnecting) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        LOG_TRACE << "SO_ERROR = " << err << " " << strerror(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    sockets::close(sockfd);
    setState(States::kDisconnected);
    if (m_connect) {
        LOG_INFO << "Connector::retry - Retry connecting to " << m_serverAddr.toIpPort()
                 << " in " << m_retryDelayMs << " milliseconds. ";

        m_loop->runAfter(m_retryDelayMs / 1000.0, [this]() { startInLoop(); });

        m_retryDelayMs = std::min(m_retryDelayMs * 2, kMaxRetryDelayMs);
    } else {
        LOG_DEBUG << "do not connect";
    }
}

int Connector::removeAndResetChannel()
{
    m_channel->disableAll();
    m_channel->remove();

    int sockfd = m_channel->getFd();
    m_loop->queueInLoop([this]() { resetChannel(); });

    return sockfd;
}

void Connector::resetChannel()
{
    m_channel.reset();
}