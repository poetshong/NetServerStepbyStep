#include "TcpServer.h"
#include "EventLoop.h"

#include <iostream>

using std::placeholders::_1;
using std::placeholders::_2;
using std::cout;

TcpServer::TcpServer(EventLoop* loop, const Address& localAddr):
    nextConnIdx_(1),
    loop_(loop),
    ipPort_(localAddr.port2string()),
    serverAcceptor_(std::make_unique<Acceptor>(loop, localAddr))
{
    serverAcceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    
}

void TcpServer::start()
{
    serverAcceptor_->listen();
}

void TcpServer::newConnection(int sockfd, const Address& peerAddr)
{
    // newConnection established called. Execute by Acceptor::accept()
    Address localAddr(localAddr_);
    std::string name = ipPort_ + "-" + std::to_string(nextConnIdx_);
    TcpConnectionPtr connection(new TcpConnection(
        loop_, name, sockfd, localAddr, peerAddr
    ));
    cout << "TcpServer::newConnection: [" << name << "] established\n";
    connection->setConnectionCallback(connectionCallback_);
    connection->setMessageCallback(messageCallback_);
    connection->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    
    ++nextConnIdx_;
    connection->connectionEstablished();
    tcpConnections_[name] = connection;  
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    cout << "TcpServer::removeConnection() [" << conn->name() << "]" << std::endl;
    // conn->connectionDestroy();
    cout << "TcpServer::removeConnection() erase connection [" << conn->name() << "]" << std::endl;
    tcpConnections_.erase(conn->name());   
}