#pragma once

#include "Acceptor.h"
#include "Channel.h"
#include "Callbacks.h"

#include <memory>
#include <functional>
#include <string>
#include <map>

class Acceptor;
class TcpConnection;

class TcpServer
{
public:
    TcpServer(EventLoop* loop, const Address& localAddr);
    ~TcpServer();

    void setConnectionCallback(ConnectionCallback cb) { connectonCallback_ = cb; }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = cb; }

    void newConnection(int sockfd, const Address& peerAddr);
    void removeConnection(TcpConnection* conn);
    
    void start();
private:
    int nextConnIdx_;
    const std::string ipPort_;
    EventLoop* loop_;
    std::unique_ptr<Acceptor> serverAcceptor_;
    Address localAddr_;
    MessageCallback messageCallback_;
    ConnectionCallback connectonCallback_;
    std::map<std::string, std::unique_ptr<TcpConnection>> tcpConnections_;
};