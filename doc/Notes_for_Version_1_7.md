# Notes for Version 1.7
A net library with based-object programming
1. using epoll I/O multiplexing
2. using class to manager resources
3. based on a basic Reactor
4. owning application level buffers per connection
5. owning Timer and TimerQueue
6. Multi-thread / ThreadPool
一个使用 I/O 复用及基于对象特性的可以处理定时时间的 Reactor 范式 Multi-thread 网络库

- [Notes for Version 1.7](#notes-for-version-17)
- [Improvements](#improvements)
- [Bugs and Summary](#bugs-and-summary)
- [Reference](#reference)

# Improvements
线程池是一个生产消费者问题

进程/线程通信问题
pipe/eventfd/条件变量
eventfd比pipe更加高效，少用一个fd，同时缓冲区只有定长 8 bytes，综合进 epoll 统一处理

让I/O线程也能执行一些计算任务(doPendingfunctors)

# Bugs and Summary


# Reference
- [1] 陈硕. Linux多线程服务端编程——使用muduo C++网络库. 2013-01