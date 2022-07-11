#pragma once 

#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <functional>

#include "Timestamp.h"
#include "Callbacks.h"

class Epoller;
class Channel;
class TimerQueue;

class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    void loop();
    void updateChannel(Channel* channel);
    // void removeChannel(Channel* channel);
    void quit();

    void runAt(Timestamp when, TimeCallback cb);
    void runAfter(Microsecond delayMill, TimeCallback cb);
    void runEvery(Microsecond intervalMill, TimeCallback cb);
    void cancelTimer(TimerId timerId);

    // void functorEnqueue(Functor func);
private:
    // void doPendingFunctor();

    std::atomic_bool isLooping_;
    std::atomic_bool quit_;

    const std::thread::id pthreadID_;   // for one loop per thread
    
    std::unique_ptr<Epoller> poller_;
    std::vector<Channel*> activeChannels_;
    
    std::unique_ptr<TimerQueue> timerQueue_;
    // std::vector<Functor> pendingFunctors_;
};