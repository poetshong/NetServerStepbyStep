#include "Thread.h"

#include <assert.h>
#include <sys/syscall.h>    // for SYS_gettid
#include <unistd.h> // for syscall

Thread::Thread(ThreadFunc func, const std::string& name):
    started_(false),
    joined_(false),
    name_(name),
    threadFunc_(std::move(func)),
    thread_(&Thread::run, this),
    pthreadId_(thread_.get_id())
{
    
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        thread_.detach();
    }
}

void Thread::join()
{
    assert(started_ && !joined_);
    joined_ = true;
    thread_.join();
}

bool Thread::joined() const
{
    return thread_.joinable();
}

bool Thread::started() const
{
    assert(!started_);
    return started_;
}


std::thread::id Thread::getpthreadID() const
{
    return pthreadId_;
}

void Thread::start()
{
    assert(!started_);
    std::unique_lock<std::mutex> lock(mutex_);
    started_ = true;
    cv_.notify_one();
}

void Thread::run()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!started_)
        {
            cv_.wait(lock);
        }
    }
    if (threadFunc_)    // default ctor threadFunc is nullptr which cannot be called
    {
        threadFunc_();
    }
}