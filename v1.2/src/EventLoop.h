#pragma once

#include "Sockets.h"

#include <vector>

class Epoller;
class Channel;

class EventLoop
{
public:
    EventLoop(Epoller* epoller);
    
    void loop();
private:
    bool isLooping_;
    Epoller* epoller_;
    std::vector<Channel*> activeChannels_;
};