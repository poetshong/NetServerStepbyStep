#pragma once

#include <netinet/in.h>
#include <string>
// #include <sys/socket.h>

class Address
{
public:
    Address(int port = 0, std::string ip = "127.0.0.1");
    Address(const struct sockaddr_in& addr):sockAddress_(addr){ }
    Address(const Address& addr);

    const int getPort() const;
    const std::string port2string() const;
    socklen_t* addrLen() { return (socklen_t*)&addrLen_; }

    struct sockaddr* getSockAddress() const 
    { return (struct sockaddr*)&sockAddress_; }

private:
    uint32_t addrLen_;
    struct sockaddr_in sockAddress_;
};
