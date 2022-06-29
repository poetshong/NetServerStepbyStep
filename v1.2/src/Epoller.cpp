#include "Epoller.h"
#include "Address.h"

#include <unistd.h>
#include <assert.h>
#include <cstring>
#include <iostream>

Epoller::Epoller(const Sockets&socket):serverSock_(socket),
                                epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
                                listenEvents_(MaxEventNumber)
{
    assert(epollFd_ != -1);
    int fd = socket.fd();
    registerFd(fd, [this]{ acceptConnection();} );
}

Epoller::~Epoller()
{
    ::close(epollFd_);
}

void Epoller::poll(std::vector<Channel*>* activeChannels, int timeoutMiliseconds)
{
    int activeEventsNumber = ::epoll_wait(epollFd_, listenEvents_.data(), 
                                        MaxEventNumber, timeoutMiliseconds);
    printf("Wait for %d\n", timeoutMiliseconds);
    if (activeEventsNumber < 0)
    {
        // error
    }
    
    for (int i = 0; i < activeEventsNumber; ++i)
    {
        Channel* channel = &channelsMap[listenEvents_[i].data.fd];
        channel->setRevents(listenEvents_[i].events);
        activeChannels->push_back(channel);
    }
}

void Epoller::registerFd(int fd, ReadCallback callback)
{
    printf("Register fd %d\n", fd);
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    ::epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event);
    Channel channel(fd);
    channel.setReadCallback(callback);
    channelsMap.insert(std::make_pair(channel.fd(), channel));
}

void Epoller::clearBuf()
{
    ::bzero(buffer_, BufferSize);
}


void Epoller::onMessage(int fd)
{
    clearBuf();
    int recvLen = recv(fd, buffer_, BufferSize - 1, 0);
    if (recvLen < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // error
        }
        channelsMap.erase(fd);
        ::close(fd);
    }
    else if (recvLen == 0)
    {
        printf("Connect from fd - %d closed.\n", fd);
        channelsMap.erase(fd);
        ::close(fd);
    }
    else
    {
        printf("Get data: %s", buffer_);
        ::send(fd, buffer_, recvLen, 0);
    }
    
}

void Epoller::acceptConnection()
{
    Address clientAddres;
    int clientSock = serverSock_.accept(&clientAddres);
    printf("Accept fd %d\n", clientSock);
    registerFd(clientSock, [this, clientSock]{ onMessage(clientSock); });
}