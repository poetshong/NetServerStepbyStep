#pragma once

#include <chrono>

using namespace std::literals;
using Millisecond = std::chrono::milliseconds;
using Microsecond = std::chrono::microseconds;
using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

using TimerId = size_t;

static Timestamp now()
{
    return std::chrono::system_clock::now();
}

static Timestamp nowAfter(Microsecond interval)
{
    return now() + interval;
}

