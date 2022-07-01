# Notes for Version 1.4
An improved EchoServer with based-object programming
1. using epoll I/O multiplexing
2. using class to manager resources
3. based on a basic Reactor
4. multi-thread
一个使用 I/O 复用及基于对象特性的 Reactor 多线程 Echo 服务器
- [Notes for Version 1.4](#notes-for-version-14)
- [Basics](#basics)
  - [condition_variable](#condition_variable)
- [Thread类](#thread类)
  - [测试](#测试)
- [Reference](#reference)
# Basics
## condition_variable
`condition_variable` 是同步原语，能用于阻塞一个线程或同时阻塞多个线程，直至另一线程修改共享变量（条件）并通知 `condition_variable`

一般和 `std::lock_guard` 配合使用

如果要修改条件变量，步骤如下<sup>[5]</sup>：
1. 获得 `std::mutex` （ `std::lock_guard` ）
2. 在保有锁时进行修改
3. 在 `std::condition_variable` 上执行 `notify_one 或 notify_all`

见 `Thread` 类的 `start()` 函数

而对于等待在条件变量上的线程，步骤如下<sup>[5]</sup>
1. 检查条件( `while` )，是否为已更新或提醒它的情况
2. 执行 `wait` 、 `wait_for` 或 `wait_until` ，等待操作自动释放互斥，并悬挂线程的执行。
3. `condition_variable` 被通知时，时限消失或虚假唤醒发生，线程被唤醒，且自动重获得互斥。之后线程应检查条件，若唤醒是虚假的，则继续等待。

见 `Thread` 类的 `run()` 函数
# Thread类
`C++11` 的 `std::thread` 已经有较好的封装，在这里通过互斥锁( `std::mutex` )和条件变量( `condition_variable` )控制线程开始执行
```c++
class Thread
{
public:
    using ThreadFunc = std::function<void()>;
    Thread(ThreadFunc func = nullptr, const std::string& name = "thread");
    ~Thread();
    void start();
    void run();
private:
    ThreadFunc threadFunc_;
    std::thread thread_;
    std::thread::id pthreadId_; // thread id
    std::condition_variable cv_;
    std::mutex mutex_;
};
```
相比 `std::thread` 创建直接开始执行，当 `Thread` 被创建时，执行 `run()` ，而 `run()` 会等待条件变量
```c++
void Thread::run()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!started_)   // while 见 [3][Spurious wakeup](https://en.wikipedia.org/wiki/Spurious_wakeup)
        {
            cv_.wait(lock);
        }
    }
    if (threadFunc_)    // default ctor threadFunc is nullptr which cannot be called
    {
        threadFunc_();
    }
}
```
调用 `start()` 时，会调用该类的 `cv_` 的 `notify_one()` ，唤醒 `wait` 在该条件变量上的线程，由于等待在 `cv_` 上的只有该线程，因此会唤醒自己，进而执行 `threadFunc()`
```c++
void Thread::start()
{
    assert(!started_);
    std::unique_lock<std::mutex> lock(mutex_);
    started_ = true;
    cv_.notify_one();
}
```
此处使用 `while` 等待条件的原因是 `Spurious wakeup` ，在[Wiki](https://en.wikipedia.org/wiki/Spurious_wakeup)<sup>[3]</sup>上和 【Programming with Posix Threads】的 p80 进行了阐述
> Wiki:
> Because spurious wakeups can happen whenever there's a race and possibly even in the absence of a race or a signal, when a thread wakes on a condition variable, it should always check that the condition it sought is satisfied. 

> Programming with Posix Threads:
> the wait may (occasionally) return when no thread specifically broadcast or
signaled that condition variable

此外
```c++
while (!started_)
{
   cv_.wait(lock);
}
```
还有另一种写法
```c++
cv_.wait(lock, []{ return started; });
```
实际上 `cv_.wait(lock, condition)` 等价于<sup>[6]</sup>：
```c++
while (!condition()) {
    wait(lock);
}
```

## 测试
以下为 `Thread` 类的测试代码
```c++
class Foo
{
public:
    void operator()()
    {
        std::cout << "Foo   tid: " << std::this_thread::get_id() << std::endl;
    }
};

void func1()
{
    std::cout << "func1 tid: " << std::this_thread::get_id() << std::endl;
}

void func2()
{
    std::cout << "func2 tid: " << std::this_thread::get_id() << std::endl;
}

int main()
{
    std::cout << "main  tid: " << std::this_thread::get_id() << std::endl;

    // 如果一个线程先结束，它的线程 id 可能会复用
    // 调用 join 会阻塞当前的线程，直到调用它的线程结束执行
    Thread t1(func1, "t1");
    Thread t2(func2);
    t1.start();
    t2.start();
    
    std::cout << "Thrd1 tid: "  << t1.getpthreadID() << " in main\n";
    std::cout << "Thrd2 tid: "  << t2.getpthreadID() << " in main\n";

    t1.join();
    t2.join();

    Thread t3([]{ std::cout << "func3 tid: " << std::this_thread::get_id() << std::endl; });
    t3.start();
    Thread t4([]{ std::cout << "func4 tid: " << std::this_thread::get_id() << std::endl; });
    t4.start();

    t3.join();
    t4.join();

    Foo f;
    Thread t5(f);
    t5.start();
    t5.join();
    std::cout << "Thrd5 tid: "  << t5.getpthreadID() << " in main\n";
}
```
测试代码一种可能的执行结果为：
```bash
main  tid: 139738504591168
Thrd1 tid: 139738504587008 in main
Thrd2 tid: 139738496194304 in main
func2 tid: 139738496194304
func1 tid: 139738504587008
func3 tid: 139738496194304
func4 tid: 139738504587008
Foo   tid: 139738504587008
Thrd5 tid: 139738504587008 in main
```
以上结果可以发现出现了重复的线程id，这是因为一个线程调用 `join` 后，主线程会阻塞直到调用它的线程执行结束，而一个线程执行结束后，它的线程id可能被重复使用

将上述代码所有 `join()` 的调用放在最后，即可观察得到不同的线程id
```bash
main  tid: 139726711387968 # 主线程线程 id
Thrd1 tid: 139726711383808 in main 
Thrd2 tid: 139726702991104 in main
func2 tid: 139726702991104
func1 tid: 139726711383808
func3 tid: 139726694598400
func4 tid: 139726686205696
Thrd5 tid: 139726677812992 in main
Foo   tid: 139726677812992
```

# Reference
- [1] 陈硕. Linux多线程服务端编程——使用muduo C++网络库. 2013-01
- [2] [cppreference/join](https://zh.cppreference.com/w/cpp/thread/thread/join)
- [3] [Spurious wakeup](https://en.wikipedia.org/wiki/Spurious_wakeup)
- [4] David R. Butenhof. Programming with POSIX Threads. 1996
- [5] [cppreference/condition_variable](https://en.cppreference.com/w/cpp/thread/condition_variable)
- [6] [cppreference/condition_variable/wait](https://en.cppreference.com/w/cpp/thread/condition_variable/wait)