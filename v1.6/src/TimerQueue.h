#pragma once

#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

#include <atomic>
#include <assert.h>
#include <set>
#include <memory>
#include <vector>
#include <unordered_map>
#include <sys/timerfd.h>

class Timer
{
    static std::atomic<TimerId> nextId_;
    static TimerId nextId();
public:
    Timer(TimeCallback cb, Timestamp when, Microsecond interval);
    ~Timer();

    bool isCanceled() const { return canceled_; }
    bool needRepeated() const { return repeated_; }
    Timestamp when() const { return timing_; }
    TimerId id() const { return timerId_; }
    bool expired(Timestamp now) const { return now >= timing_; }

    void run()
    {
        printf("Timer [%ld] execute callback\n", timerId_);
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
        canceled_ = true;
    }

private:
    TimeCallback timeCallback_;
    Timestamp timing_;
    Microsecond interval_;
    bool repeated_;
    bool canceled_;
    const TimerId timerId_;
};

class EventLoop;

class TimerQueue
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    Timer* addTimer(TimeCallback cb, Timestamp when, Microsecond interval);
    void cancelTimer(Timer* timer);

    int64_t firstExpiredTimer() const;
private:
    using Entry = std::pair<Timestamp, Timer*>;
    using Timers = std::set<Entry>;

    void handleTimingRead();

    std::vector<Entry> getExpired(Timestamp current);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    Timers timerList_;
};