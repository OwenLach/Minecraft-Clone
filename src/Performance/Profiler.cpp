#include "Performance/Profiler.h"

#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <numeric>
#include <iostream>

Profiler::Profiler()
{
}
Profiler::~Profiler()
{
}

Profiler &Profiler::get()
{
    static Profiler instance;
    return instance;
}

void Profiler::record(const std::string &name, const double duration)
{
    std::unique_lock<std::mutex> lock(timingsMutex_);
    timings_[name].push_back(duration);
}

void Profiler::renderStats()
{
    std::unique_lock<std::mutex> lock(timingsMutex_);
    for (const auto &[name, times] : timings_)
    {
        double avg = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        std::cout << name << ": " << avg << "ms" << std::endl;
    }

    timings_.clear();
};
