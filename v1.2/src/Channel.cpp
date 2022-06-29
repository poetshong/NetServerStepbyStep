#include "Channel.h"

#include <sys/epoll.h>

const int Channel::ReadEvent = EPOLLIN;
const int Channel::NoEvent = 0;

Channel::Channel(int fd):fd_(fd)
{

}

// void Channel::update()
// {

// }

void Channel::handleEvents()
{
    // not worked yet
    if ( !(revents_ & EPOLLIN))
    {
        printf("Excute closeCallback\n");
        if (closeCallback_)
        {
            closeCallback_();
        }
    }

    // work
    if (revents_ & EPOLLIN)
    {
        printf("Excute readCallback\n");
        if (readCallback_)
        {
            readCallback_();
        }
    }
    
}