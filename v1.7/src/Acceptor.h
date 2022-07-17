#pragma once

#include "Sockets.h"
#include "Address.h"
#include "Channel.h"

#include <atomic>
#include <functional>

class EventLoop;

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int, const Address&)>;

    Acceptor(EventLoop* loop, const Address& localAddress);

    void listen();
    void setNewConnectionCallback(NewConnectionCallback cb) { newConnectionCallback_ = cb; }

    struct sockaddr_in getLocalAddress(int sockfd);
private:
    void accept();
    std::atomic_bool listenning_;

    EventLoop* loop_;
    Sockets acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
};