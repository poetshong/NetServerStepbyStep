#include "Channel.h"
#include "EventLoop.h"

#include <sys/epoll.h>
#include <assert.h>

const int Channel::NoEvent = 0;
const int Channel::ReadableEvent = EPOLLIN;
const int Channel::WritableEvent = EPOLLOUT;


Channel::Channel(EventLoop* loop, int fd):
    loop_(loop),
    fd_(fd), 
    events_(0), 
    revents_(0),
    status_(-1),
    closeEventCallback_(nullptr),
    readEventCallback_(nullptr),
    writeEventCallback_(nullptr)
{

}

void Channel::handleEvents()
{
    if (revents_ & EPOLLHUP)    // events analysis
    {
        if (closeEventCallback_)
        {
            closeEventCallback_();
        }
    }

    if (revents_ & EPOLLIN)
    {
        if (readEventCallback_)
        {
            readEventCallback_();
        }
    }

    if (revents_ & EPOLLOUT)
    {
        if (writeEventCallback_)
        {
            writeEventCallback_();
        }
    }
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    assert(isNoneEvent());
    loop_->removeChannel(this);
}