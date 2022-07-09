#pragma once

#include "Acceptor.h"
#include "Channel.h"
#include "Callbacks.h"
#include "TcpConnection.h"

#include <memory>
#include <string>
#include <map>

class Acceptor;

class TcpServer
{
public:
    TcpServer(EventLoop* loop, const Address& localAddr);
    ~TcpServer();

    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = cb; }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = cb; }

    void newConnection(int sockfd, const Address& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    
    void start();
private:
    int nextConnIdx_;
    const std::string ipPort_;
    EventLoop* loop_;
    std::unique_ptr<Acceptor> serverAcceptor_;
    Address localAddr_;
    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    std::map<std::string, std::shared_ptr<TcpConnection>> tcpConnections_;
};