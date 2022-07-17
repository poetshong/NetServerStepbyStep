#include "../src/TcpServer.h"
#include "../src/EventLoop.h"
#include "../src/Address.h"

#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

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
        const auto no{std::chrono::system_clock::now()};
        const auto curr{std::chrono::system_clock::to_time_t(no)};
        std::stringstream ss;
        ss << std::put_time(std::localtime(&curr), "%Y/%m/%d %I:%M:%S %p\n");
        conn->send(ss.str());
        conn->shutdown();   // bug: socket shutdown errono 107
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
    TcpServer server(&loop, server_address);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    server.start();

    loop.loop();
}