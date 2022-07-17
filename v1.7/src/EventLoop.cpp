#include "EventLoop.h"
#include "Epoller.h"
#include "Channel.h"
#include "TimerQueue.h"

#include <assert.h>
#include <iostream>

// __thread EventLoop* thread_EventLoop = NULL;

EventLoop::EventLoop():
    isLooping_(false),
    quit_(false),
    pthreadID_(std::this_thread::get_id()),
    poller_(std::make_unique<Epoller>(this)),
    timerQueue_(new TimerQueue(this))
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
        // int timeoutMill = static_cast<int>(timerQueue_->firstExpiredTimer());
        int timeoutMill = -1;
        // printf("EventLoop::polling() wait %d milliseconds...\n", timeoutMill);
        poller_->poll(timeoutMill, activeChannels_);

        for (auto activeChannel: activeChannels_)
        {
            // printf("EventLoop::polling() handleEvents\n");
            activeChannel->handleEvents();
        }
        // printf("EventLoop::doPendingFunctor()\n");
        // doPendingFunctor();
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

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

// void EventLoop::functorEnqueue(Functor fun)
// {
//     pendingFunctors_.push_back(std::move(fun));
// }

// void EventLoop::doPendingFunctor()
// {
//     for (const auto & func: pendingFunctors_)
//     {
//         func();
//     }
// }

void EventLoop::runAt(Timestamp when, TimeCallback cb)
{
    printf("EventLoop::runAt()\n");
    timerQueue_->addTimer(cb, when, std::chrono::microseconds::zero());
}

void EventLoop::runAfter(Microsecond delayMicro, TimeCallback cb)
{
    printf("EventLoop::runAfter()\n");
    // Timestamp t = nowAfter(delayMicro);
    runAt(delayMicro + now(), cb);
}

void EventLoop::runEvery(Microsecond intervalMicro, TimeCallback cb)
{
    printf("EventLoop::runEvery()\n");
    Timestamp t = nowAfter(intervalMicro);
    timerQueue_->addTimer(cb, t, intervalMicro);
}

void EventLoop::cancelTimer(Timer* timer)
{
    timerQueue_->cancelTimer(timer);
}