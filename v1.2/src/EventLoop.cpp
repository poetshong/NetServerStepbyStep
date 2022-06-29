#include "EventLoop.h"
#include "Epoller.h"


EventLoop::EventLoop(Epoller* epoller):epoller_(epoller)
{

}

void EventLoop::loop()
{
    isLooping_ = true;
    while (isLooping_)
    {
        activeChannels_.clear();
        int timeout = -1;
        epoller_->poll(&activeChannels_, timeout);
        for (auto& channel: activeChannels_)
        {
            printf("ActiveChannel Size: %d\n", activeChannels_.size());
            channel->handleEvents();
        }
    }
    isLooping_ = false;
}