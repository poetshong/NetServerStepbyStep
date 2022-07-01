#pragma once

#include <functional>

class TcpConnection;

using MessageCallback = std::function<void(const TcpConnection*, char* buf, int len)>;
using ConnectionCallback = std::function<void(const TcpConnection*)>;
using CloseCallback = std::function<void(TcpConnection*)>;
using ReadEventCallback = std::function<void()>;
using WriteEventCallback = std::function<void()>;
using CloseEventCallback = std::function<void()>;


void defaultConnectionCallback(const TcpConnection* conn);

void defaultMessageCallback(const TcpConnection* conn, char* buf, int len);