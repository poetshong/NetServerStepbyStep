#include "../src/Thread.h"

#include <iostream>

#include <sys/syscall.h>    // for SYS_gettid
#include <unistd.h> // for syscall
#include <assert.h>

// std::hash<std::thread::id>{}(std::this_thread::get_id()) 将 tid 转换为 ld 

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
    // https://zh.cppreference.com/w/cpp/thread/thread/join
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