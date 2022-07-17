#include "TimerQueue.h"
#include "EventLoop.h"


#include <cstring>
#include <unistd.h>
#include <iostream>

static int createTimerfd()
{
    int fd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0)
    {
        printf("Create timerfd failed\n");
        abort();
    }
    return fd;
}

static void readTimerfd(int timerfd)
{
    // for read timerfd to avoid triggering always
    // printf("ReadTimerfd()\n");
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    // printf("n = %ld; sizeof buf = %lu\n", n, sizeof(buf));
    assert(n == sizeof(howmany));
    // Log
}

static struct timespec durationFromNow(Timestamp when)
{
    // printf("DurationFromNow()\n");
    auto microseconds = std::chrono::duration_cast<Microsecond>(when - now());
    if (microseconds < 1ms)
    {
        microseconds = 1ms;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds.count() / std::nano::den);
    ts.tv_nsec = static_cast<long>((microseconds.count() % std::nano::den));
    return ts;
}

static void setTimerfd(int timerfd, Timestamp ts)
{
    // printf("SetTimerfd()\n");
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(itimerspec));
    bzero(&oldValue, sizeof(itimerspec));
    newValue.it_value = durationFromNow(ts);
    
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret < 0)
    {
        printf("SetTimerfd() timerfd_settime error\n");
        abort();
    }
}

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
    printf("Timer %ld is created\n", timerId_);
}

Timer::~Timer()
{
    printf("Timer %ld is destroyed\n", timerId_);
}

TimerQueue::TimerQueue(EventLoop* loop):
    loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_)
{
    assert(timerfd_ > 0);
    printf("TimerQueue::TimerQueue() timerfd: [%d]\n", timerfd_);
    timerfdChannel_.setReadableCallback([this] { handleTimingRead(); });
    timerfdChannel_.enabledReadable();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.diabledAllEvents();
    timerfdChannel_.remove();
    ::close(timerfd_);
        for (auto& timer: timerList_)
    {
        delete timer.second;
    }
}

Timer* TimerQueue::addTimer(TimeCallback cb, Timestamp when, Microsecond interval)
{
    printf("TimerQueue::addTimer()\n");

    Timer* timer = new Timer(cb, when, interval);
    auto ret = timerList_.insert({when, timer});
    if (timerList_.begin() == ret.first)
    {
        setTimerfd(timerfd_, when);
    }
    return timer;
}

void TimerQueue::cancelTimer(Timer* timer)
{
    // printf("TimerQueue::cancelTimer()\n");
    timer->cancel();
    timerList_.erase({timer->when(), timer});
    delete timer;
}

void TimerQueue::handleTimingRead()
{
    readTimerfd(timerfd_);

    Timestamp curr(now());
    std::vector<Entry> expired = getExpired(curr);
    // printf("TimerQueue::handleTimingRead()\n");
    for (auto& e: expired)
    {
        Timer* timer = e.second;
        assert(timer->expired(curr));
        if (!timer->isCanceled())
        {
            timer->run();
        }

        if (!timer->isCanceled() && timer->needRepeated())
        {
            timer->restart();
            e.first = timer->when();
            timerList_.insert(e);
        }
        else
        {
            delete timer;
        }
    }

    if (!timerList_.empty())
    {
        setTimerfd(timerfd_, timerList_.begin()->first);
    }
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp curr)
{ 
    Entry entry(curr + 1ns, reinterpret_cast<Timer*>(UINTPTR_MAX));
    std::vector<Entry> expired;

    auto end = timerList_.lower_bound(entry);
    assert(end == timerList_.end() || curr < end->first);
    expired.assign(timerList_.begin(), end);
    // printf("TimerQueue::getExpired() [%lu] timer timeout\n", expired.size());
    timerList_.erase(timerList_.begin(), end);

    return expired;
}

int64_t TimerQueue::firstExpiredTimer() const
{
    // printf("TimerQueue::firstExpiredTimer()\n");
    if (timerList_.empty())
    {
        return 0;
    }
    auto interval = timerList_.begin()->first - now();
    return std::chrono::duration_cast<Millisecond>(interval).count();
}