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
    EventLoop();
    ~EventLoop();
    void loop();
    void updateChannel(Channel* channel);
    void quit();
private:
    std::atomic_bool isLooping_;
    std::atomic_bool quit_;

    const std::thread::id pthreadID_;   // for one loop per thread
    
    std::unique_ptr<Epoller> poller_;
    std::vector<Channel*> activeChannels_;
};