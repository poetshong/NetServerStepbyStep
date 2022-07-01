#pragma once 

#include <memory>
#include <vector>
#include <atomic>
#include <thread>

class Epoller;
class Channel;

class EventLoop
{
public:
    // using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    void loop();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void quit();

    // void functorEnqueue(Functor func);
private:
    // void doPendingFunctor();

    std::atomic_bool isLooping_;
    std::atomic_bool quit_;

    const std::thread::id pthreadID_;   // for one loop per thread
    
    std::unique_ptr<Epoller> poller_;
    std::vector<Channel*> activeChannels_;
    // std::vector<Functor> pendingFunctors_;
};