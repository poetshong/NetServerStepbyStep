#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>

class Thread
{
public:
    using ThreadFunc = std::function<void()>;

    Thread(ThreadFunc func = nullptr, const std::string& name = "thread");
    ~Thread();
    void join();
    void start();
    void run();

    bool started() const;
    bool joined() const;
    std::thread::id getpthreadID() const;
private:
    bool started_;
    bool joined_;
    std::string name_;
    ThreadFunc threadFunc_;
    std::thread thread_;
    std::thread::id pthreadId_;
    std::condition_variable cv_;
    std::mutex mutex_;
};