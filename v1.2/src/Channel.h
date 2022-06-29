#pragma once


/*
 * Channel
 * 1. 管理文件描述符对应的回调函数
 *      - 服务器的文件描述符回调函数为 接受连接
 *      - client 的描述符回调函数为 回射消息
 * 2. 接受连接时，会生成对应的文件描述符 fd ，需要根据该 fd 构造 Channel 
 *      - 构造 Channel 后注册对应的回调函数
 *      - 将文件描述符加入到 epollfd 的监听中
 * 3. 断开连接时也要有对应的回调函数，然后析构对应的 channel 
 * 4. 负责执行回调函数
 * 5. 什么时候创建？哪个类管理？
 * 
*/

#include <functional>

class Channel
{
public:
    using ReadCallback = std::function<void()>;
    using CloseCallback = std::function<void()>;
    // using ConnectionCallback = std::function<void()>;

    Channel() = default;
    Channel(int fd);
    
    int fd() { return fd_; }
    void setReadCallback(const ReadCallback& cb) { readCallback_ = cb; }    // 可读事件的回调函数 not used yet
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; } // 断开连接的回调函数
    // void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }  // 建立连接的回调函数

    void setRevents(uint32_t events) { revents_ = events; }
    // void update();
    void handleEvents(); // execute callbacks
private:    
    const static int ReadEvent;
    const static int NoEvent;

    ReadCallback readCallback_;
    CloseCallback closeCallback_;
    // ConnectionCallback connectionCallback_;

    int revents_;
    int events_;
    int fd_;
};