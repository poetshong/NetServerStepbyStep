#include <sys/socket.h>
#include <assert.h>
#include <unistd.h>
#include <cstring>

#include "Sockets.h"

Sockets::Sockets():fd_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
{
    assert(fd_>= 0);
    setReuseAddr();
}

Sockets::~Sockets()
{
    close();
}

void Sockets::setReuseAddr()
{
    int val = 1;
    int ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    assert(ret != -1);
}

void Sockets::bind(const Address& localAddress)
{
    // note: bind(fd, ...) the fd is not equal the return value
    int ret = ::bind(fd_, localAddress.getSockAddress(), 
                    static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    assert(ret != -1);
}

void Sockets::listen()
{
    // note: listen(fd, ...) the fd is not equal the return value
    printf("listen...\n");
    int ret = ::listen(fd_, 16);
    assert(ret != -1);
}

int Sockets::accept(Address* peerAddress)
{
    // note: accept4(fd, ...) the fd is not equal the return value
    printf("accept...\n");
    int clientFd = ::accept4(fd_, peerAddress->getSockAddress(), peerAddress->addrLen(),
                    SOCK_CLOEXEC | SOCK_NONBLOCK);
    assert(clientFd > 0);
    return clientFd;
}

void Sockets::close()
{
    ::close(fd_);
}

