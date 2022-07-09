#pragma once

#include <netinet/in.h>

#include "Address.h"

class Sockets
{
public:
    explicit Sockets();
    ~Sockets();

    int fd() const { return fd_; }

    void bind(const Address& localAddress);
    void listen();

    void setReuseAddr();

    int accept(Address *peerAddress);

    struct sockaddr_in getLocalAddr(int sockfd);

    void shutdownWrite();
    void close();
private:
    int fd_;
};
