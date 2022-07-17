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
    void removeChannel(Channel* chanenl);
private:
    static const int NEW = -1;
    static const int ADDED = 1;
    static const int DEL = 2;
    void updateChannel(Channel* channel, int op);

    const static int DeaultEventNums = 128;
    
    EventLoop* loop_;
    int epollfd_;
    std::vector<epoll_event> activeEvents_;
};