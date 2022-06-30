#pragma once

#include <functional>

class EventLoop;

class Channel
{
public:
    using ReadEventCallback = std::function<void()>;
    using WriteEventCallback = std::function<void()>;
    using CloseEventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);

    int fd() const { return fd_; }

    void setReturnEvents(int revents) { revents_ = revents; }
    void setEvents(int events) { events_ = events; };
    int events() { return events_; }
    bool isNoneEvent() const { return events_ == NoEvent; }
    bool isPolling() const { return polling_; }
    void setPolling(bool on) { polling_ = on; }

    void enabledReadable() { events_ |= ReadableEvent; update(); }
    void enabledWritable() { events_ |= WritableEvent; update(); }
    
    void diabledAllEvents() { events_ = NoEvent; update(); }
    void disabledReadable() { events_ &= ~ReadableEvent; update(); }
    void disabledWritable() { events_ &= ~WritableEvent; update(); }

    void setReadableCallback(ReadEventCallback cb) { readEventCallback_ = std::move(cb); }
    void setWritableCallback(WriteEventCallback cb) { writeEventCallback_ = std::move(cb); }
    void setCloseEventCallback(CloseEventCallback cb) { closeEventCallback_ = std::move(cb); }
    void handleEvents();
private:
    const static int ReadableEvent;
    const static int WritableEvent;
    const static int NoEvent;

    void update();

    int fd_;
    int events_;
    int revents_;
    bool polling_;
    EventLoop* loop_;
    WriteEventCallback writeEventCallback_;
    ReadEventCallback readEventCallback_;
    CloseEventCallback closeEventCallback_;
};