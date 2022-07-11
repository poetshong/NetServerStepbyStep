#include "TimerQueue.h"
#include "EventLoop.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <iostream>

std::atomic<TimerId> Timer::nextId_;

TimerId Timer::nextId()
{
    ++nextId_;
    return nextId_;
}

Timer::Timer(TimeCallback cb, Timestamp when, Microsecond interval):
    timeCallback_(cb),
    timing_(when),
    interval_(interval),
    repeated_(interval > Microsecond::zero()),
    canceled_(false),
    timerId_(nextId())
{

}

TimerQueue::TimerQueue(EventLoop* loop):
    loop_(loop),
    timerfd_(::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC)),
    timerfdChannel_(loop, timerfd_),
    timerList_()
{
    if (timerfd_ < 0)
    {
        // error
        abort();
    }
    timerfdChannel_.setReadableCallback([this]{ this->handleTimingRead(); });
    timerfdChannel_.enabledReadable();
    printf("TimerQueue::TimerQueue() timerfd: [%d]\n", timerfd_);
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.diabledAllEvents();
    loop_->updateChannel(&timerfdChannel_);
    ::close(timerfd_);
}

void TimerQueue::addTimer(TimeCallback cb, Timestamp when, Microsecond interval)
{
    printf("TimerQueue::addTimer()\n");
    std::unique_ptr<Timer> newTimer = std::make_unique<Timer>(cb, when, interval);
    timerIdList_[newTimer->id()] = newTimer.get();
    timerList_[when] = std::move(newTimer);
}

void TimerQueue::cancelTimer(TimerId timerId)
{
    printf("TimerQueue::cancelTimer()\n");
    Timer* timer = timerIdList_[timerId];
    if (timer->expired())
    {
        timerIdList_.erase(timerId);
    }
    else
    {
        timer->cancel();
    }
}

void TimerQueue::handleTimingRead()
{
    Timestamp current(now());

    std::vector<std::unique_ptr<Timer>> expired = getExpired(current);
    printf("TimerQueue::handleTimingRead()\n");
    for (auto& timer: expired)
    {
        timer->run();
        if (timer->needRepeated() && !timer->isCanceled())
        {
            timer->restart();
            timerList_[timer->when()] = std::move(timer);
        }
        else
        {
            TimerId id = timer->id();
            timerIdList_.erase(id);
        }
    }

}

std::vector<std::unique_ptr<Timer>> TimerQueue::getExpired(Timestamp current)
{
    printf("TimerQueue::getExpired()\n");
    std::vector<std::unique_ptr<Timer>> expired;
    auto end = timerList_.lower_bound(current);
    assert(end == timerList_.end() || current < end->first);
    for (auto iter = timerList_.begin(); iter != end; ++iter)
    {
        expired.push_back(std::move(iter->second));
    }
    timerList_.erase(timerList_.begin(), end);
    return expired;
}

int TimerQueue::firstExpiredTimer() const
{
    printf("TimerQueue::firstExpiredTimer()\n");
    if (timerList_.empty())
    {
        return 0;
    }
    auto diffTime = timerList_.begin()->first - now();
    return std::chrono::duration_cast<Millisecond>(diffTime).count();
}