# Notes for Version 1.6
A net library with based-object programming
1. using epoll I/O multiplexing
2. using class to manager resources
3. based on a basic Reactor
4. owning application level buffers per connection
5. owning Timer and TimerQueue
一个使用 I/O 复用及基于对象特性的可以处理定时时间的 Reactor 范式 single-thread 网络库

- [Notes for Version 1.6](#notes-for-version-16)
- [Improvement](#improvement)
- [Bugs and Summary](#bugs-and-summary)
- [Reference](#reference)

# Improvement


# Bugs and Summary

`timerfd` 会出现一直触发 `epoll_wait` 的情况，即 `epoll_wait` 会瞬间返回
在类似的 [libnet]() 项目中也有该情况，muduo 却没有出现该情况

[C++ multiple definition](https://www.jianshu.com/p/c028fee0f202)

# Reference
- [1] 陈硕. Linux多线程服务端编程——使用muduo C++网络库. 2013-01