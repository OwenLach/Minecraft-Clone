#pragma once

#include <chrono>
#include <string>

class ScopedTimer
{
public:
    ScopedTimer(const std::string &name);
    ~ScopedTimer();

private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;
};