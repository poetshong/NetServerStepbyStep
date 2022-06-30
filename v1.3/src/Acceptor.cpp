#include "Acceptor.h"

#include <assert.h>
#include <unistd.h>

Acceptor::Acceptor(EventLoop* loop, const Address& localAddress):
    loop_(loop),
    listenning_(false),
    acceptSocket_(),
    acceptChannel_(loop, acceptSocket_.fd())
{
    acceptSocket_.bind(localAddress);
    acceptChannel_.setReadableCallback(std::bind(&Acceptor::accept, this));
}


void Acceptor::listen()
{
    assert(!listenning_);
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enabledReadable();
}

void Acceptor::accept()
{
    Address peerAddress;
    int clientfd = acceptSocket_.accept(&peerAddress);
    printf("client fd: %d\n", clientfd);
    if (clientfd > 0)
    {
        if (newConnectionCallback_)
        {
            printf("Run newConnectionCallback\n");
            newConnectionCallback_(clientfd, peerAddress);
        }
    }
    else
    {
        ::close(clientfd);
    }
}