#include "../src/TcpServer.h"
#include "../src/EventLoop.h"
#include "../src/Address.h"
#include "../src/TcpConnection.h"

#include <iostream>

#include <unistd.h>
#include <cstring>

void newConnection(const TcpConnection* conn)
{
    printf("newConnection(): Connection [%s]: (%d->%d) is %s\n", 
        conn->name().c_str(),
        conn->localAddress().getPort(), 
        conn->peerAddress().getPort(),
        (conn->connected()? "UP": "DOWN"));
    
}

void onMessage(const TcpConnection* conn, const char* data, int n)
{
    std::cout << "onMessage(): receive " << n << " bytes from connection [" << conn->name() << "]\n";
}

int main()
{
    int port = 9190;
    Address server_address(port);
    EventLoop loop;
    TcpServer server(&loop, server_address);
    server.setConnectionCallback(newConnection);
    server.setMessageCallback(onMessage);

    server.start();
    
    loop.loop();
}