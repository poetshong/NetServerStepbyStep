# Notes for Version 1
Start with a simple echo server with epoll
以一个简单的使用多路复用 epoll 的 Echo Server 开始
- [Notes for Version 1](#notes-for-version-1)
- [Echo 服务器](#echo-服务器)
- [Basic](#basic)
- [Epoll](#epoll)
- [服务器逻辑](#服务器逻辑)
- [Others](#others)
- [References](#references)
# Echo 服务器
Echo 服务器：将任何接收到的数据原封不动地转发回去
服务器功能的基本逻辑如下：
```
while (true)
{
    建立连接;
    接收数据;
    发送数据;
    断开连接;
}
```
# Basic
从服务端的角度分析，给定客户端，要想与服务端通信，客户端必须知道服务器所在的 『`IP` 地址』以及『端口』，因此首先创建两个变量，代表服务器的地址
```c++
const char* server_ip = "127.0.0.1"; // server ip. Here is loop address
int server_port = 9019; // 0~1023 is not for users
```
我们在应用层使用套接字(socket)与传输层沟通。

然后使用 `socket` 函数创建套接字，函数返回“套接字描述符”，用于应用层与传输层的数据传输
```c++
// #include <sys/socket.h>
int server_sock = socket(AF_INET, SOCK_STREAM, 0);  // (1)
assert(server_sock >= 0); // (2)
```

(1) `socket` 接受三个参数，第一个参数指定协议族(protocol/address family)，`PF_XXX` 或者 `AF_XXX` ，第二个参数指定套接字类型(type)，如果是 `TCP` 使用 `SOCK_STREAM` (字节流套接字)，`UDP` 使用 `SOCK_DGRAM` (数据报套接字)，第三个参数通常设为 `0`，表示默认协议
> 目前协议族和地址族是等价的，即 `AF_INET` 和 `PF_INET` 可以混用

对于 `type`，还可以接受标识：`SOCK_NONBLOCK` 和 `SOCK_CLOXEC`，分别代表创建非阻塞 `socket` 和 `fork` 时在子进程关闭该 `socket`

(2) 判断套接字是否创建成功，失败会返回 `-1`

接着，设置网络套接字地址：
```c++
// #include <netinet/in.h>
struct sockaddr_in server_addr; // (3) IPv4 套接字地址结构
bzero(&server_addr, sizeof server_addr); // (4)
server_addr.sin_family = AF_INET; // (5) 
server_addr.sin_port = htons(server_port); // (6)
server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // (7)
```
(3)-(4) 创建 `socket` 专用地址，并初始化
(5) 设置套接字地址族
(6) 设置套接字端口，对于网络传输来说，需要通过 `htons` 将端口转换成网络字节序
(7) 设置 IP 地址结构体，`INADDR_ANY` 表示该服务器计算机上所有网卡的IP都能作为服务器的IP地址，代表“本机”

再然后，绑定以及监听套接字：
```c++
int ret = bind(server_sock, (struct sockaddr*)&server_addr, sizeof server_addr); // (8)
assert(ret != -1);
ret = listen(server_sock, 5);
assert(ret != -1);
```
(8) 需要将 `sockaddr_in` 转换为 `sockaddr` 
> 系统调用Listen() 中 backlog 的含义：
> 1. Linux协议栈维护的TCP连接有两个队列：SYN 半连接队列和 accept 队列
> 2. 服务器收到客户端的 SYN 并回复之后，该连接信息就被移到一个队列中，这个队列就是 SYN 半连接队列
> 3. 当服务器收到客户端的 SYN/ACK 包之后，就将该连接从半连接队列移动到另一个队列，这个队列就是 accept 队列「此时三次连接已经完成」
> 当调用 accept() 之后，该连接信息就会从 accept 队列移走
> backlog 在Linux2.2之后仅代表后者的大小

以上步骤就创建了 `socket` 地址，下面以 `epoll` 为例阐述多路复用的基本 API
# Epoll
`I/O` 复用的目的：让程序同时对多个文件描述符进行监听/处理
由主要使用三个 `API` 
```c++
#include <sys/epoll.h>
int epoll_create(int size); //epfd事件表
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event); //成功返回0，失败返回-1，设置errno

struct epoll_event {
    __uint32_t events;  //epoll事件
    epoll_data_t data;  //用户数据
};

typedef union epoll_data {  //联合体，
    void* ptr;
    int fd; //需要监听的套接字
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);   //成功返回就绪的描述个数，失败返回-1，设置errno
//epoll_wait() returns the number of file descriptors ready for the requested I/O,
```
`size` 告知内核事件表多大，但是内核会忽略；
`fd`: 要操作的文件描述符
`maxevents`: 最多监听的事件的个数
`op`: 指定操作类型
- `EPOLL_CTL_ADD`，在事件表中注册fd上要关注的事件
- `EPOLL_CTL_MOD`，修改fd上的注册事件
- `EPOLL_CTL_DEL`，删除fd上的注册事件

那么代码如下：
```c++
int setNonBlocking(int fd) // (12)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd) // (13)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN; // EPOLLET (14)
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

// create epollfd
// different between epoll_create and epoll_create1
int epollfd = ::epoll_create1(EPOLL_CLOEXEC); // (9)
assert(epollfd != -1);

const int MaxEventNumber = 1024;    // (10)
std::vector<epoll_event> events(MaxEventNumber); // (11)
```
(12) 将文件描述符设置为非阻塞
(13) 将文件添加到 epoll 的监听列表中
(14) 如果设置 EPOLLET 则会使用 epoll 的 ET 触发模式，默认为 LT触发
> epoll_create(int size) 和 epoll_create1(int flag) 的区别，前者传递预计监听树大小(但是会被忽略)，后者传递描述符属性

# 服务器逻辑
回到 Echo 服务器的基本逻辑，即实现建立连接、收发数据的逻辑
```c++
while (true)
    {
        int active_event = ::epoll_wait(epollfd, events.data(), MaxEventNumber, -1); // (15)
        if (active_event < 0)
        {
            // error
        }
        for (int i = 0; i < active_event; ++i) // (16)
        {
            int active_sockfd = events[i].data.fd; // (17)
            if (active_sockfd == server_sock)
            {
                connect_sock = accept4(server_sock, (struct sockaddr*)&client_addr, 
                    &client_addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);    // (18)
                addfd(epollfd, connect_sock);   // (19)
            }
            else if (events[i].events & EPOLLIN)    // (20)
            {
                  bzero(buffer, Buffer_Size);
                  ret = recv(active_sockfd, buffer, Buffer_Size - 1, 0);    // (21)
                  if (ret < 0)
                  {
                      // error 
                      ::close(connect_sock);
                  }
                  else if (ret == 0)
                  {
                      ::close(active_sockfd);   // (22)
                  }
                  else
                  {
                      ::send(active_sockfd, buffer, ret, 0); // (23)
                  }
            }
            else
            {
                // something else
            }
        }
    }
```
(15) IO复用的基本思想是将所有会引起阻塞的操作都集中至 epoll_wait 中，当事件被激活时逐个处理事件
> epoll_wait 的 timeout 单位为毫秒，如果设置为特殊值 -1 代表事件没有事件激活的时候一直阻塞，0 代表立即返回；函数返回活动的文件描述符的个数，并将活动的事件放入 events 中

(16) 遍历所有活动的事件
(17) 获得活动事件的文件描述符
(18) 如果是服务器对应的文件描述符(server_sock)可读，说明有新的连接，这里使用 accept4() 接受连接
(19) 给新建立的连接注册可读事件
(20) 如果是已有连接对应的文件描述符可读，往后进入处理逻辑
(21) 将文件描述符中的数据读入 buffer，recv() 返回从文件描述符中读取的字节长度
(22) 如果读入数据长度为 0，说明连接已断开，关闭连接的套接字
(23) 如果读入了数据，根据 echo 服务器的逻辑，将直接将原数据写入连接的文件描述符

# Others
对于单个文件的编译和运行，可以使用脚本处理
（在使用脚本之前注意修改文件权限 `chmod u+x run.sh` ）
```bash
# !bin/bash
g++ EchoServer.cpp -o EchoServer
./EchoServer
```

另一个测试 `Echo` 服务器是否正常工作，可以使用命令 `nc` 连接端口，发送字符消息
```bash
nc 127.0.0.1 9190
```
然后发送字符，断开连接，即可测试服务是否正常

# References
- [1] Steve. UNIX网络编程 vol.1 套接字联网API
- [2] 游双. Linux高性能服务器编程. 2013-06
- [3] 陈硕. Linux多线程服务端编程——使用muduo C++网络库. 2013-01
- [4] capxchen. [从零开始的C++网络编程](https://mp.weixin.qq.com/s/VdJHh_5C-DUs0hSFqp8j0A). 2019-10
- [5] morganhuang. [彻底弄懂TCP协议：从三次握手说起](https://mp.weixin.qq.com/s/6LiZGMt2KRiIoMaLwx-lkQ)