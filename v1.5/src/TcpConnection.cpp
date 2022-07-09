#include "TcpConnection.h"
#include "EventLoop.h"
#include "Address.h"
#include "Channel.h"

#include <assert.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
using std::cout;

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    printf("Connection [%s]: (%d->%d) is %s\n", 
        conn->name().c_str(),
        conn->localAddress().getPort(), 
        conn->peerAddress().getPort(),
        (conn->connected()? "UP": "DOWN"));
}

void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buf)
{
    printf("defaultMessageCallback(): onMessage(): received %ld bytes for connection [%s]\n",
            buf->readableSize(), conn->name().c_str());
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
    channel_->setWritableCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseEventCallback(std::bind(&TcpConnection::handleClose, this));
}

void TcpConnection::connectionEstablished()
{
    printf("TcpConnection::connectionEstablished()\n");
    assert(connectState_ == CONNECTING);
    setConnectState(CONNECTED);
    channel_->enabledReadable();
    
    connectionCallback_(shared_from_this());
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
    printf("TcpConnection::handleRead()\n");
    ssize_t n = inputBuffer_.readFd(channel_->fd());
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    else if (n == 0)
    {
        handleClose();
    }
}

void TcpConnection::handleWrite()
{
    printf("TcpConnection::handleWrite()\n");
    if (channel_->isWriting())
    {
        ssize_t n = ::write(channel_->fd(),
                            outputBuffer_.readIndexPtr(),
                            outputBuffer_.readableSize());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableSize() == 0)
            {
                channel_->disabledWritable();
                
                // ？？
                if (connectState_ == DISCONNECTING)
                {
                    shutdown();
                }
            }
        }
        else
        {
            printf("TcpConnection::handleWrite() error/n");
            abort();
        }

    }
    else
    {
        printf("Connection is down, no more writing/n");    
    }
}

void TcpConnection::handleClose()
{
    cout << "TcpConnection::handleClose() Connection [" << name_ << "] from port:[" << peerAddr_.port2string() << "]\n";
    setConnectState(DISCONNECTED);
    channel_->diabledAllEvents();
    connectionCallback_(shared_from_this());
    closeCallback_(shared_from_this());
}

void TcpConnection::shutdown()
{
    if (connectState_ != CONNECTED && !channel_->isWriting())
    {
        setConnectState(DISCONNECTING);
        socket_->shutdownWrite();
    }
}

void TcpConnection::send(const std::string& message)
{
    printf("TcpConnection::send(string&)\n");
    ssize_t nwrote = 0;
    // channel has no writing event
    // output buffer has send all data
    // write data directly
    if (!channel_->isWriting() && outputBuffer_.readableSize() == 0)
    {

        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote >= 0)
        {
            // log
        }
        else
        {
            nwrote = 0;
            // error
        }

    }

    assert(nwrote >= 0);
    if (static_cast<size_t>(nwrote) < message.size())
    {
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel_->isWriting())
        {
            channel_->enabledWritable();
        }
    }
}
