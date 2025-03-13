#include "Acceptor.h"
#include "Util.h"
#include "Logger.h"

using namespace inet;

Acceptor::Acceptor(EventLoop *eventloop, const InetAddress &listenAddr) :
    m_loop(eventloop),
    m_acceptSocket(Socket()),
    m_acceptChannel(m_loop, m_acceptSocket.fd()),
    m_listen(false)
{
    sockets::setReuseAddr(m_acceptSocket.fd());
    m_acceptSocket.bind(listenAddr);
    m_acceptChannel.setReadCallback([this]() { handleRead(); });
}

Acceptor::~Acceptor()
{
    m_acceptChannel.disableAll();
    m_acceptChannel.remove();
}

void Acceptor::listen()
{
    m_acceptSocket.listen();
    m_acceptChannel.enableReading();
    m_listen = true;
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = m_acceptSocket.accept(&peerAddr);
    if (connfd >= 0) {
        if (m_newConnectionCallback) {
            m_newConnectionCallback(connfd, peerAddr);
        }
    } else {
        LOG_ERROR << "accpet error";
    }
}