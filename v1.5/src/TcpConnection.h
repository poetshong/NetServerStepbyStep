#pragma once

#include "Address.h"
#include "Sockets.h"
#include "Callbacks.h"

#include <memory>
#include <string>

class EventLoop;
class Channel;

class TcpConnection
{
public:
    enum ConnectState {CONNECTED, CONNECTING, DISCONNECTED};

    TcpConnection(EventLoop* loop, const std::string name, int clientfd, const Address& localaddr, const Address& peerAddr);

    void setMessageCallback(MessageCallback cb) { messageCallback_ = cb; };
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = cb; }
    void setCloseCallback(CloseCallback cb) { closeCallback_ = cb; }

    void connectionEstablished();   // 连接建立 connection established
    void connectionDestroy();

    void setConnectState(ConnectState cs) { connectState_ = cs; }

    Address localAddress() const { return localAddr_; }
    Address peerAddress() const { return peerAddr_; }

    bool connected() const { return connectState_ == CONNECTED; }
    bool disconnected() const { return connectState_ == DISCONNECTED; }

    std::string name() const { return name_; }
private:
    void handleRead();
    void handleClose();

    std::string name_;
    std::unique_ptr<Sockets> socket_;
    std::unique_ptr<Channel> channel_;
    Address localAddr_;
    Address peerAddr_;
    EventLoop* loop_;
    ConnectState connectState_;
    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    CloseCallback closeCallback_;
};