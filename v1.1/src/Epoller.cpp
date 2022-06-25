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
    registerFd(socket.fd());
}

Epoller::~Epoller()
{
    ::close(epollFd_);
}

void Epoller::poll(int timeoutMiliseconds)
{
    int activeEventsNumber = ::epoll_wait(epollFd_, listenEvents_.data(), 
                                        MaxEventNumber, timeoutMiliseconds);
    // printf("Epoll wait return...\n");
    if (activeEventsNumber < 0)
    {
        // error
    }

    for (int i = 0; i < activeEventsNumber; ++i)
    {
        // Sockets
        // Address
        int activeSockfd = listenEvents_[i].data.fd;
        if (activeSockfd == serverSock_.fd())
        {
            Address clientAddres;
            printf("Accept before\n");
            int clientSock = serverSock_.accept(&clientAddres);
            // add clientSock to epollfd
            registerFd(clientSock);
        }
        else if (listenEvents_[i].events & EPOLLIN)
        {
            clearBuf();
            int recvLen = recv(activeSockfd, buffer_, BufferSize - 1, 0);
            if (recvLen < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // error
                }
                ::close(activeSockfd);
            }
            else if (recvLen == 0)
            {
                ::close(activeSockfd);
            }
            else
            {
                ::send(activeSockfd, buffer_, recvLen, 0);
            }
        }
    }
}

void Epoller::registerFd(int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    ::epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event);
}

void Epoller::clearBuf()
{
    ::bzero(buffer_, BufferSize);
}

