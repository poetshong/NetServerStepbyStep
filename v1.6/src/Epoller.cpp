#include "Epoller.h"
#include "Channel.h"

#include <assert.h>
#include <unistd.h>
#include <cstring>

Epoller::Epoller(EventLoop* loop):
    loop_(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    activeEvents_(DeaultEventNums)
{
    assert(epollfd_ >= 0);
}

Epoller::~Epoller()
{
    ::close(epollfd_);
}

void Epoller::poll(int timeoutMill, std::vector<Channel*>& activeChannels)
{

    int activeEventsNum = ::epoll_wait(epollfd_, 
                                        activeEvents_.data(), 
                                        static_cast<int>(activeEvents_.size()), 
                                        timeoutMill);
    // printf("Epoller::poll() %d events happened\n", activeEventsNum);
    if (activeEventsNum > 0)
    {

        for (int i = 0; i < activeEventsNum; ++i)
        {
            Channel* channel = static_cast<Channel*>(activeEvents_[i].data.ptr);
            // printf("Active fd is [%d]\n", channel->fd());
            printf("event is %d\n", channel->returnEvents());
            channel->setReturnEvents(activeEvents_[i].events);
            activeChannels.push_back(channel);
        }

        if (static_cast<size_t>(activeEventsNum) == activeEvents_.size())
        {
            activeEvents_.resize(activeEvents_.size() * 2);
        }
    }
    else if (activeEventsNum == 0)
    {
        printf("Epoller::poll() nothing happened\n");
    }
    else
    {
        printf("Epoller::poll() activeEventsNum < 0 error\n");
    }
}

void Epoller::updateChannel(Channel* channel)
{
    int op = 0;
    int st = channel->getStatus();
    if (st == NEW || st == DEL)
    {
        // a new channel
        printf("Epoller::updateChannel() Update EPOLL_CTL_ADD...\n");
        op = EPOLL_CTL_ADD;
        channel->setStatus(ADDED);
        
    }
    else if (!channel->isNoneEvent())
    {
        printf("Epoller::updateChannel() Update EPOLL_CTL_MOD...\n");
        op = EPOLL_CTL_MOD;
    }
    else
    {
        printf("Epoller::updateChannel() Update EPOLL_CTL_DEL...\n");
        op = EPOLL_CTL_DEL;
        channel->setStatus(DEL);
        
    }
    updateChannel(channel, op);
}

void Epoller::updateChannel(Channel* channel, int op)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    printf("Epoller::updateChannel() [%d] fd update\n", fd);
    if (::epoll_ctl(epollfd_, op, fd, &event) < 0)
    {
        abort();    // for debug
    }
}

void Epoller::removeChannel(Channel* channel)
{
    assert(channel->isNoneEvent());
    int st = channel->getStatus();
    if (st == ADDED)
    {
        updateChannel(channel, EPOLL_CTL_DEL);
    }
    channel->setStatus(NEW);
}
