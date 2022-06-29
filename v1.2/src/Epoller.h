#pragma once

#include "Sockets.h"
#include "Channel.h"

#include <vector>
#include <sys/epoll.h>
#include <map>

class Epoller
{
public:
    using ReadCallback = std::function<void()>;
    using CloseCallback = std::function<void()>;

    Epoller();
    Epoller(const Sockets&socket);
    ~Epoller();
    void poll(std::vector<Channel*>* activeChannels, int timeout);
    
    void registerFd(int fd, ReadCallback callback);

    void onMessage(int fd);   // 回射消息回调函数
    void acceptConnection();

    void clearBuf();
private:
    const static int MaxEventNumber = 1024;
    const static int BufferSize = 4096;

    char buffer_[BufferSize];
    Sockets serverSock_;
    int epollFd_;
    std::vector<epoll_event> listenEvents_;
    std::map<int, Channel> channelsMap;
};