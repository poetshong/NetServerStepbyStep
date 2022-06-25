#include "Address.h"
#include "Sockets.h"
#include "Epoller.h"
#include "EventLoop.h"

#include <iostream>

int main()
{
    int port = 9190;
    Address server_address(port);
    Sockets sock;
    sock.bind(server_address);
    sock.listen();
    
    Epoller epoller(sock);
    EventLoop loop(&epoller);
    loop.loop();
}