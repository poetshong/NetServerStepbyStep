#pragma once

#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

#include <atomic>
#include <assert.h>
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>

class Timer
{
    static std::atomic<TimerId> nextId_;
    static TimerId nextId();
public:
    Timer(TimeCallback cb, Timestamp when, Microsecond interval);

    bool isCanceled() const { return canceled_; }
    bool needRepeated() const { return repeated_; }
    Timestamp when() const { return timing_; }
    TimerId id() const { return timerId_; }
    bool expired() const { return now() >= timing_; }

    void run()
    {
        if (timeCallback_)
        {
            timeCallback_();
        }
    }

    void restart()
    {
        assert(repeated_);
        timing_ += interval_;
    }

    void cancel()
    {
        assert(!canceled_);
        canceled_.exchange(true);
    }

private:
    Timestamp timing_;
    TimeCallback timeCallback_;
    Microsecond interval_;
    std::atomic_bool repeated_;
    std::atomic_bool canceled_;
    const TimerId timerId_;
};

class EventLoop;

class TimerQueue
{
    using Timers = std::map<Timestamp, std::unique_ptr<Timer>>;

public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    void addTimer(TimeCallback cb, Timestamp when, Microsecond interval);
    void cancelTimer(TimerId timerId);

    int firstExpiredTimer() const;
private:
    std::vector<std::unique_ptr<Timer>> getExpired(Timestamp now);
    
    void handleTimingRead();

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    Timers timerList_;
    std::unordered_map<TimerId, Timer*> timerIdList_;
};