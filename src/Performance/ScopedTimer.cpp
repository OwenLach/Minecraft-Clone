#include "Performance/ScopedTimer.h"
#include "Performance/Profiler.h"

#include <chrono>

ScopedTimer::ScopedTimer(const std::string &name)
    : name_(name), start_(std::chrono::high_resolution_clock::now())
{
}

ScopedTimer::~ScopedTimer()
{
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double, std::milli>(end - start_).count();
    Profiler::get().record(name_, duration);
}
