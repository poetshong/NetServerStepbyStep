#include "Epoller.h"
#include "Channel.h"

#include <assert.h>
#include <unistd.h>
#include <cstring>

Epoller::Epoller(EventLoop* loop):
    loop_(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    activeEvents_(MaxEventNums)
{
    assert(epollfd_ >= 0);
}

Epoller::~Epoller()
{
    ::close(epollfd_);
}

void Epoller::poll(int timeoutMill, std::vector<Channel*>& acitiveChannels)
{

    int activeEventsNum = ::epoll_wait(epollfd_, 
                                        activeEvents_.data(), 
                                        static_cast<int>(activeEvents_.size()), 
                                        timeoutMill);
    for (int i = 0; i < activeEventsNum; ++i)
    {
        Channel* channel = static_cast<Channel*>(activeEvents_[i].data.ptr);

        channel->setReturnEvents(activeEvents_[i].events);
        acitiveChannels.push_back(channel);
    }
}

void Epoller::updateChannel(Channel* channel)
{
    int op = 0;
    if (!channel->isPolling())
    {
        printf("Epoller::updateChannel() Update EPOLL_CTL_ADD...\n");
        op = EPOLL_CTL_ADD;
        channel->setPolling(true);
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
        channel->setPolling(false);
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
    if (::epoll_ctl(epollfd_, op, fd, &event) < 0)
    {
        abort();    // for debug
    }
}
