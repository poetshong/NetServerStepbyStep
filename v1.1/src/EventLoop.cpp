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
        int timeout = 1000;
        epoller_->poll(timeout);
    }
    isLooping_ = false;
}