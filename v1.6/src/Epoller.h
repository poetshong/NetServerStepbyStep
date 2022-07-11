#pragma once

#include <vector>

#include <sys/epoll.h>

class Channel;
class EventLoop;

class Epoller
{
public:
    Epoller(EventLoop* loop);
    ~Epoller();
    void poll(int timeout, std::vector<Channel*>& activeChannels);
    void updateChannel(Channel* channel);
    // void removeChannel(Channel* chanenl);
private:
    void updateChannel(Channel* channel, int op);

    const static int MaxEventNums = 128;
    
    EventLoop* loop_;
    int epollfd_;
    std::vector<epoll_event> activeEvents_;
};