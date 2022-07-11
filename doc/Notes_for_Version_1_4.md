# Notes for Version 1.4
A daytime Server with based-object programming
1. using epoll I/O multiplexing
2. using class to manager resources
3. based on a basic Reactor
一个使用 I/O 复用及基于对象特性的具有 Reactor 的 single-thread discard 服务器

- [Notes for Version 1.4](#notes-for-version-14)
- [Improvement](#improvement)
- [Test](#test)
  - [业务逻辑 Business Logic](#业务逻辑-business-logic)
  - [程序内存泄漏 Memory Leak](#程序内存泄漏-memory-leak)
- [Other](#other)
- [Bugs and Summary](#bugs-and-summary)
- [Reference](#reference)

# Improvement
Ver1.3 中使用了 `Acceptor` 接受连接，实现了一个简单的 `daytime` 服务器

实际上 Ver1.3 连接后回复时间然后立即断开连接，无法处理连接收发数据的需求，Ver1.4 将首先实现接收数据方面的逻辑，具体涉及到 `TcpConnection` 和 `TcpServer` 的实现<sup>[1]</sup>

`TcpConnection` 为“连接”对象，对于 `TCP` 连接来说，需要包含目标地址和本地地址，然后连接建立后，`Acceptor` 会返回一个客户端的文件描述符

根据之前版本的分析，文件描述符与两个类直接相关，`Sockets` 和 `Channel`，前者管理文件描述符的生命周期，后者负责相关事件的注册、分发和回调，那么就可以基本确定 `TcpConnection` 的数据成员：
```c++
class TcpConnection
{
private:
    std::unique_ptr<Sockets> socket_;
    std::unique_ptr<Channel> channel_;
    Address localAddr_;
    Address peerAddr_;
    EventLoop* loop_; // for Channel ctor
    ConnectState connectState_;

    // Callbacks
    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    CloseCallback closeCallback_;
};
```
`Sockets` 和 `Channel` 由智能指针 `std::unique_ptr` 管理。因为 `TcpConnection` 生命周期结束后，对应的文件描述符应该关闭，因此 `TcpConnection` 也要负责它们的 `Sockets` 和 `Channel` 的析构，使用智能指针能防止内存泄漏

另外增加了枚举类型 `enum ConnectState` 来表示 `TcpConnection` 连接所处的状态

那么当连接建立时，`TcpConnection` 应该通过 `Channel` 注册自己的可读事件等，然后执行连接建立时的回调函数

```c++
void TcpConnection::connectionEstablished()
{
    assert(connectState_ == CONNECTING);
    setConnectState(CONNECTED);
    channel_->enabledReadable();    // register read events

    connectionCallback_(this);
}
```
当连接可读的时候，读取数据，并执行消息处理的回调函数
```c++
void TcpConnection::handleRead()
{
    char buf[65536];
    bzero(buf, sizeof(buf));
    ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
    if (n > 0)
    {
        messageCallback_(this, buf, n);
    }
    else if (n == 0)
    {
        handleClose();  // #(1)
    }
}
```
(1) 表示当 `::read()` 返回 0 时，对端已经断开连接，因此执行处理关闭连接的逻辑
```c++
void TcpConnection::handleClose()
{
    cout << "TcpConnection::handleClose() Connection [" << name_ << "] from port:[" << peerAddr_.port2string() << "]\n";
    setConnectState(DISCONNECTED); // (2)
    channel_->diabledAllEvents();   // (3)
    connectionCallback_(this);  // (4)
    closeCallback_(this);   // (5)
}
```
(2) 将连接状态设置为关闭连接
(3) 将注册的所有事件取消
(4) (5) 执行连接处理回调函数 `connectionCallback_()` 和关闭连接回调函数 `closeCallback_()` 

为了达成上述逻辑，需要 `handleRead()` 和 `handleClose` 在连接初始化的时候注册到 `Channel` 的回调函数中

接下来是 `TcpServer` 的实现。`TcpServer` 管理所有连接，是 `Acceptor` 进一步封装，它的功能主要有以下几个：
1. 负责 `TcpConnection` 的创建
2. 负责 `TcpConnection` 的析构
3. 管理 `Acceptor` 的生存期

`TcpServer` 要负责管理 `TcpConnection` 的生命期，要将外部用户传入的回调函数(连接建立、消息处理等回调函数)传递给 `TcpConnection`

具体来说，当 `TcpServer` 初始化时，需要注册 `Acceptor` 建立连接的回调函数，并且 `Acceptor` 可读时，也就是有新连接时，需要创建 `TcpConnection`。为了方便管理 `TcpConnection` ，使用 `std::map<string, std::unique_ptr<TcpConnection>>` ，每一个连接都有一个对应的名字，方便查找，根据上述分析，`TcpServer` 的数据成员如下：
```c++
class TcpServer
{
private:
    // for name
    int nextConnIdx_;
    const std::string ipPort_;

    EventLoop* loop_;
    std::unique_ptr<Acceptor> serverAcceptor_;
    Address localAddr_;
    MessageCallback messageCallback_;
    ConnectionCallback connectonCallback_;
    std::map<std::string, std::unique_ptr<TcpConnection>> tcpConnections_;
};
```
实现方面有以下几个要点：
1. 初始化时，设置 `Acceptor` 建立连接的回调函数：对应构造函数
2. 建立连接的回调函数需要创建 `TcpConnection` 对象，并设置对应的 `TcpConnection` 建立连接、消息处理、断开连接的回调函数：对应 `TcpServer::newConnection()` 
3. 连接断开时，需要销毁 `TcpConnection` 对象：对应 `TcpServer::removeConnection()`
4. 调用 `start()` 时开始监听端口：对应 `TcpServer::start()` 

它们的具体实现此处不再赘述

下面是建立连接时序图：
![建立连接时序图](./img/Server_v1_4_connection.png)

下面是断开连接的时序图：
![断开连接时序图](./img/Server_v1_4_closeConnection.png)

# Test
该版本有两个测试角度
1. 服务器逻辑
2. 服务器是否存在内存泄漏（虽然使用了智能指针，但是某些代码中显式调用了 `new` ）

## 业务逻辑 Business Logic
逻辑如下：
```c++
// ServerTest.cpp
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
```
运行程序，仍然使用 `nc` 命令连接到服务器，并发送两次字符串 `abcdefg` 和 `hijklmnop`，然后断开连接，服务器输出结果如下：
```bash
listen...
EventLoop::updateChannel() updateChannel
Epoller::updateChannel() Update EPOLL_CTL_ADD...
EventLoop::polling()----------------------
accept...
Acceptor::accept() client fd: 5
Acceptor::accept(): Run newConnectionCallback
TcpServer::newConnection: [9190-1] established
EventLoop::updateChannel() updateChannel
Epoller::updateChannel() Update EPOLL_CTL_ADD...
newConnection(): Connection [9190-1]: (0->47900) is UP
EventLoop::polling()----------------------
onMessage(): receive 8 bytes from connection [9190-1]
EventLoop::polling()----------------------
onMessage(): receive 10 bytes from connection [9190-1]
EventLoop::polling()----------------------
TcpConnection::handleClose() Connection [9190-1] from port:[47900]
EventLoop::updateChannel() updateChannel
Epoller::updateChannel() Update EPOLL_CTL_DEL...
newConnection(): Connection [9190-1]: (0->47900) is DOWN
TcpServer::removeConnection() [9190-1]
TcpServer::removeConnection() erase connection [9190-1]
EventLoop::polling()----------------------
```
说明程序执行的逻辑和之前的分析一致
## 程序内存泄漏 Memory Leak
使用 `valgrind` 检查内存泄漏：
```bash
valgrind --leak-check=full --show-leak-kinds=all ./build/bin/ServerTest
```
以下为存在的报错/警告信息：
```bash
==5236== Invalid read of size 4
==5236==    at 0x10FA6C: Channel::handleEvents() (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10D5AA: EventLoop::loop() (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10BCE2: main (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==  Address 0x4dd0b38 is 8 bytes inside a block of size 120 free'd
# ...
==5236== Process terminating with default action of signal 2 (SIGINT)
# ...
==5236== HEAP SUMMARY:
==5236==     in use at exit: 1,808 bytes in 6 blocks
==5236== 8 bytes in 1 blocks are still reachable in loss record 1 of 6
# ...
==5236==    by 0x10E29F: Epoller::poll(int, std::vector<Channel*, std::allocator<Channel*> >&) (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10D544: EventLoop::loop() (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10BCE2: main (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
# ... 
==5236== 24 bytes in 1 blocks are still reachable in loss record 3 of 6
# ...
==5236==    by 0x110473: Acceptor::setNewConnectionCallback(std::function<void (int, Address const&)>) (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10FCE8: TcpServer::TcpServer(EventLoop*, Address const&) (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10BC48: main (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
# ...
==5236== 40 bytes in 1 blocks are still reachable in loss record 4 of 6
# ...
==5236==    by 0x10D416: EventLoop::EventLoop() (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10BC28: main (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
# ...
==5236== 176 bytes in 1 blocks are still reachable in loss record 5 of 6
# ...
==5236==    by 0x10FBDB: TcpServer::TcpServer(EventLoop*, Address const&) (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10BC48: main (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)

# ...
==5236== 1,536 bytes in 1 blocks are still reachable in loss record 6 of 6
# ...
==5236==    by 0x10D416: EventLoop::EventLoop() (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
==5236==    by 0x10BC28: main (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
```
以上存在两个主要问题
1. `Invalid read of size 4`
2. `in use at exit: 1,808 bytes in 6 blocks`

对于第1个问题，说明有代码访问到了没有初始化或无效的地址，查看输出信息，发现有这么一行:
```bash
==5236==    at 0x10FA6C: Channel::handleEvents() (in /home/poetshong/Code/NetServerStepbyStep/v1.4/build/bin/ServerTest)
```
这说明在 `Channel` 内 `handleEvents()` 出现了无效访问( `invalid read` )，其对应的代码实现如下：
```c++
void Channel::handleEvents()
{
    if (revents_ & EPOLLHUP)    // events analysis
    {
        if (closeEventCallback_)
        {
            closeEventCallback_();
        }
    }

    if (revents_ & EPOLLIN)
    {
        if (readEventCallback_)
        {
            readEventCallback_();
        }
    }

    if (revents_ & EPOLLOUT)
    {
        if (writeEventCallback_)
        {
            writeEventCallback_();
        }
    }
}
```
根据 Ver1.4 的逻辑，是没有服务器发送数据相关的逻辑的，这意味着没有注册写相关的事件，`writeEventCallback_` 初始化后是个空指针，`varlgrind` 认为这是一种不合理的访问，`if (revents_ & EPOLLOUT) ...` 代码块注释后该问题消失

对于第二个问题，`valgrind` 在官方文档中指出:
> "Still reachable". This covers cases 1 and 2 (for the BBB blocks) above. A start-pointer or chain of start-pointers to the block is found. Since the block is still pointed at, the programmer could, at least in principle, have freed it before program exit. "Still reachable" blocks are very common and arguably not a problem. So, by default, Memcheck won't report such blocks individually.

这说明这是中断程序 `SIGINT` 导致主程序中未能及时释放资源导致的

# Other
该版本实现过程中，`EventLoop` 使用了 `std::unique_ptr` 管理 `Epoller`，在 `EventLoop` 的构造函数中调用了 `std::make_unique<Epoller>(this)` ，编译报错，在 Ver1.2 中使用裸指针却并没有报错，具体错误信息为：
```bash
/usr/include/c++/9/bits/unique_ptr.h: In instantiation of ‘void std::default_delete<_Tp>::operator()(_Tp*) const [with _Tp = Epoller]’:

/usr/include/c++/9/bits/unique_ptr.h:292:17:   required from ‘std::unique_ptr<_Tp, _Dp>::~unique_ptr() [with _Tp = Epoller; _Dp = std::default_delete<Epoller>]’

/home/poetshong/Code/NetServerStepbyStep/v1.4/Test/../src/EventLoop.h:11:7:   required from here

/usr/include/c++/9/bits/unique_ptr.h:79:16: error: invalid application of ‘sizeof’ to incomplete type ‘Epoller’
   79 |  static_assert(sizeof(_Tp)>0,
```
关键点是：`error: invalid application of ‘sizeof’ to incomplete type ‘Epoller’`，编译器指出，在构造函数中，使用了一个不完整的类型( `incomplete type` )

查阅资料，在《Effective Modern C++》 中 【Item 22: 使用 Pimpl 技法时，将特殊成员函数的定义放到实现文件中】<sup>[2]</sup>有分析该情况的产生原因：

`EventLoop` 离开作用域被析构时，因为没有声明析构函数，编译器会生成合成默认析构函数，在析构函数中，编译器会插入代码来调用数据成员 `poller_` ，`poller_` 是一个 `std::unique_ptr<Epoller>` 类型的对象，即一个使用了默认析构器的 `std::unique`

默认析构器在内部调用 `delete` 运算符来对裸指针指向的对象进行析构，然而在 `delete` 之前，编译器会使用 C++11 中的 `static_assert` 来确保裸指针并不是一个不完整的类型。而由于此时 `EventLoop` 的析构函数是隐式内敛的，编译器没有看到 `EventLoop` 中 `Epoller` 的定义和 `EventLoop.cpp` 自动生成的析构函数，从而导致断言(assert)失败

原文如下：
> The issue arises due to the code that’s generated when w is destroyed (e.g., goes out of scope). At that point, its destructor is called. In the class definition using std::unique_ptr, we didn’t declare a destructor, because we didn’t have any code to put into it. In accord with the usual rules for compiler-generated special member functions (see Item 17), the compiler generates a destructor for us. Within that destructor, the compiler inserts code to call the destructor for Widget’s data member pImpl. pImpl is a std::unique_ptr\<Widge::pImpl>, i.e., a std::unique_ptr using the default deleter. The default deleter is a function that uses delete on the raw pointer inside the std::unique_ptr. Prior to using delete, however, implementations typically have the default deleter employ C++11’s static_assert to ensure that the raw pointer doesn’t point to an incomplete type. When the compiler generates code for the destruction of the Widget w, then, it generally encounters a static_assert that fails, and that’s usually what leads to the error message. This message is associated with the point where w is destroyed, because Widget’s destructor, like all compiler-generated special member functions, is implicitly inline. The message itself often refers to the line where w is created, because it’s the source code explicitly creating the object that leads to its later implicit destruction.

在 `EventLoop` 中显式声明析构函数，然后在实现文件中定义析构函数即可

# Bugs and Summary
2022.06.20
* 如果 `main` 不调用 `set*Callback()`，会引起程序崩溃，但是按照设计逻辑，不调用时会有默认的回调函数
* Debug 使用了 `printf` 和 `cout` 两种风格的输出方式，代码风格不一致，将来编写日志时将会统一处理

# Reference
- [1] 陈硕. Linux多线程服务端编程——使用muduo C++网络库. 2013-01
- [2] Scott Meyers, 高博译. Effective Modern C++. 2018-04