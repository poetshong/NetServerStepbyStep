# Notes for Version 1.5
A net library with based-object programming
1. using epoll I/O multiplexing
2. using class to manager resources
3. based on a basic Reactor
4. owning application level buffers per connection
一个使用 I/O 复用及基于对象特性的具有 Reactor 的 single-thread 网络库

- [Notes for Version 1.5](#notes-for-version-15)
- [Improvement](#improvement)
- [Test](#test)
- [Bugs and Summary](#bugs-and-summary)
- [Reference](#reference)

# Improvement
在这个版本中，将实现 `Buffer` ，即应用层的输入和输出缓冲


# Test

# Bugs and Summary

[shared_from_this() causes std::bad_weak_ptr even when correctly using make_shared](https://stackoverflow.com/questions/50318414/shared-from-this-causes-stdbad-weak-ptr-even-when-correctly-using-make-share)

[how to convert chrono::time_point to string without using array?](https://stackoverflow.com/questions/35114956/how-to-convert-chronotime-point-to-string-without-using-array)

连接断开的问题
ERRONO: ENOTCONN 107 Transport endpoint is already connected.

# Reference
- [1] 陈硕. Linux多线程服务端编程——使用muduo C++网络库. 2013-01