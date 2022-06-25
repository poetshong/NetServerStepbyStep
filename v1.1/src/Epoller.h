#pragma once

#include "Sockets.h"

#include <vector>
#include <sys/epoll.h>

class Epoller
{
public:
    Epoller();
    Epoller(const Sockets&socket);
    ~Epoller();
    void poll(int timeout);
    void registerFd(int fd);

    void clearBuf();
private:
    const static int MaxEventNumber = 1024;
    const static int BufferSize = 4096;

    char buffer_[BufferSize];
    Sockets serverSock_;
    int epollFd_;
    std::vector<epoll_event> listenEvents_;
};