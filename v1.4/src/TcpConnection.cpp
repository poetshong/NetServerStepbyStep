#include "TcpConnection.h"
#include "EventLoop.h"
#include "Address.h"
#include "Channel.h"

#include <assert.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
using std::cout;

void defaultConnectionCallback(const TcpConnection* conn)
{
    printf("Connection [%s]: (%d->%d) is %s\n", 
        conn->name().c_str(),
        conn->localAddress().getPort(), 
        conn->peerAddress().getPort(),
        (conn->connected()? "UP": "DOWN"));
}

void defaultMessageCallback(const TcpConnection* conn, char* buf, int len)
{
    printf("defaultMessageCallback(): onMessage(): received %d bytes for connection [%s]\n",
            len, conn->name().c_str());
}

TcpConnection::TcpConnection(EventLoop* loop, std::string name, int clientfd, const Address& localaddr, const Address& peerAddr):
    loop_(loop),
    name_(name),
    socket_(new Sockets()),
    channel_(new Channel(loop, clientfd)),
    localAddr_(localaddr),
    peerAddr_(peerAddr),
    connectState_(CONNECTING),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback)
{
    channel_->setReadableCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setCloseEventCallback(std::bind(&TcpConnection::handleClose, this));
}

void TcpConnection::connectionEstablished()
{
    assert(connectState_ == CONNECTING);
    setConnectState(CONNECTED);
    channel_->enabledReadable();

    connectionCallback_(this);
}

// void TcpConnection::connectionDestroy()
// {
//     // if (connectState_ == CONNECTED)
//     // {
//     //     setConnectState(DISCONNECTED);
//     //     channel_->diabledAllEvents();
//     //     cout << "TcpConneticon::connectionDestroy() Connection [" << name_ << "] from port:[" << peerAddr_.port2string() << "] disconneted\n";
//     //     connectionCallback_(this);
//     // }
//     cout << "TcpConneticon::connectionDestroy() Connection [" << name_ << "] from port:[" << peerAddr_.port2string() << "] disconneted\n";
// }

void TcpConnection::handleRead()
{
    char buf[65536];
    bzero(buf, sizeof(buf));
    ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
    if (n > 0)
    {
        messageCallback_(this, buf, n);
    }
    else if (n == 0)
    {
        handleClose();
    }
}

void TcpConnection::handleClose()
{
    cout << "TcpConnection::handleClose() Connection [" << name_ << "] from port:[" << peerAddr_.port2string() << "]\n";
    setConnectState(DISCONNECTED);
    channel_->diabledAllEvents();
    connectionCallback_(this);
    closeCallback_(this);
}