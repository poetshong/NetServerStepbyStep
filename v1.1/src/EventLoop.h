#pragma once

#include "Sockets.h"

class Epoller;

class EventLoop
{
public:
    EventLoop(Epoller* epoller);
    
    void loop();
private:
    bool isLooping_;
    Epoller* epoller_;
};