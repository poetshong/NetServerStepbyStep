#include "../src/Address.h"
#include "../src/Sockets.h"
#include "../src/Epoller.h"
#include "../src/EventLoop.h"
#include "../src/Acceptor.h"

#include <iostream>

#include <unistd.h>
#include <cstring>

void newConnection(int sockfd, const Address& peerAddrsss)
{
    printf("NewConnection: accept a new connection from port[%d]\n", peerAddrsss.getPort());
    time_t t;
    time(&t);
    char* str = ctime(&t);
    char buf[64];
    bzero(buf, sizeof(buf));
    ::strcat(buf, str);
    ::write(sockfd, buf, sizeof(buf));
    ::close(sockfd);
}

int main()
{
    int port1 = 9190;
    Address server_address1(port1);
    EventLoop loop;
    Acceptor acceptor1(&loop, server_address1);
    acceptor1.setNewConnectionCallback(newConnection);
    acceptor1.listen();

    int port2 = 9192;
    Address server_address2(port2);
    Acceptor acceptor2(&loop, server_address2);
    acceptor2.setNewConnectionCallback(newConnection);
    acceptor2.listen();
    
    loop.loop();
}