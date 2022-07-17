#include "../src/EventLoop.h"
#include "../src/Channel.h"

#include <sys/timerfd.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>

EventLoop* g_loop;

void timeout()
{
    printf("Timeout\n");
    g_loop->quit();
}

int main()
{
    EventLoop loop;
    g_loop = &loop;
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    Channel channel (&loop, timerfd);
    channel.setReadableCallback(timeout);
    channel.enabledReadable();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop();
    ::close(timerfd);
    printf("Timerfd in main is [%d]\n", timerfd);
}