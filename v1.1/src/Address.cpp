#include "Address.h"

#include <cstring>

Address::Address(int port, std::string ip):addrLen_(static_cast<uint32_t>(sizeof sockAddress_))
{
    bzero(&sockAddress_, sizeof sockAddress_);
    sockAddress_.sin_family = AF_INET;
    sockAddress_.sin_port = htons(port);
    sockAddress_.sin_addr.s_addr = htonl(INADDR_ANY);
}


const int Address::getPort() const
{
    return sockAddress_.sin_port;
}

const std::string Address::port2string() const
{
    return std::to_string(ntohs(sockAddress_.sin_port));
}

