# Notes for Version 1.5
A daytime Server with based-object programming
1. using epoll I/O multiplexing
2. using class to manager resources
3. based on a basic Reactor
4. owning application level buffers per connection
一个使用 I/O 复用及基于对象特性的具有 Reactor 的 single-thread 服务器

# Improvement
在这个版本中，将实现 `Buffer` ，用于给每个连接收发数据



# Reference
- [1] 陈硕. Linux多线程服务端编程——使用muduo C++网络库. 2013-01
- [2] 