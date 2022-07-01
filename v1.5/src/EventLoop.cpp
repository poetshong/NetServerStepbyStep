#include "EventLoop.h"
#include "Epoller.h"
#include "Channel.h"

#include <assert.h>

// __thread EventLoop* thread_EventLoop = NULL;

EventLoop::EventLoop():
    isLooping_(false),
    quit_(false),
    pthreadID_(std::this_thread::get_id()),
    poller_(std::make_unique<Epoller>(this))
{
    // if (thread_EventLoop)
    // {
    //     // error
    // }
    // else
    // {
    //     thread_EventLoop = this;
    // }
    // assert(std::this_thread::get_id() == pthreadID_);
}

EventLoop::~EventLoop()
{
    // thread_EventLoop = NULL;
}

void EventLoop::loop()
{
    assert(!isLooping_);
    isLooping_.exchange(true);
    while (!quit_)
    {
        activeChannels_.clear();
        int timeoutMill = -1;
        printf("EventLoop::polling()----------------------\n");
        poller_->poll(timeoutMill, activeChannels_);

        for (auto activeChannel: activeChannels_)
        {
            activeChannel->handleEvents();
        }
    }

    isLooping_.exchange(false);
}

void EventLoop::updateChannel(Channel* channel)
{
    printf("EventLoop::updateChannel() updateChannel\n");
    poller_->updateChannel(channel);
}

void EventLoop::quit()
{
    quit_.exchange(true);
}