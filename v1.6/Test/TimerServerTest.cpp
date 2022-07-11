#include "../src/TcpServer.h"
#include "../src/EventLoop.h"
#include "../src/Address.h"

#include <iostream>
#include <chrono>

#include <unistd.h>
#include <cstring>

using namespace std::literals;

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
    std::string msg(buf->retrieveAsString());
    std::cout << conn->name() << " discards " << msg.size() << 
    " bytes received"<< std::endl;
}



int main()
{
    int port = 9190;
    Address server_address(port);
    EventLoop loop;
    loop.runAfter(1s, []{ std::cout << "once 1s" << std::endl;});
    // loop.runAfter(4s, []{ std::cout << "once 4s" << std::endl;});
    // loop.runEvery(2s, []{ std::cout << "every 2s" << std::endl;});

    TcpServer server(&loop, server_address);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    server.start();
    
    loop.loop();
}