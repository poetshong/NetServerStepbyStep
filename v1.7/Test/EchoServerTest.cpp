#include "../src/TcpServer.h"
#include "../src/EventLoop.h"
#include "../src/Address.h"

#include <iostream>

#include <unistd.h>
#include <cstring>

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        printf("newConnection(): Connection [%s]: (%d->%d) is %s\n", 
            conn->name().c_str(),
            conn->localAddress().getPort(), 
            conn->peerAddress().getPort(),
            (conn->connected()? "UP": "DOWN"));
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",
        conn->name().c_str());
    }
    
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
    printf("onMessage(): received %zd bytes from connection [%s]\n",
        buf->readableSize(),
        conn->name().c_str());
    conn->send(buf->retrieveAsString());
}

int main()
{
    int port = 9190;
    Address server_address(port);
    EventLoop loop;
    TcpServer server(&loop, server_address);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    server.start();
    
    loop.loop();
}