#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

class Profiler
{
private:
    Profiler();
    ~Profiler();

    std::unordered_map<std::string, std::vector<double>> timings_;
    std::mutex timingsMutex_;

public:
    static Profiler &get();

    void record(const std::string &name, const double duration);
    void renderStats();

    Profiler(const Profiler &) = delete;
    Profiler &operator=(const Profiler &) = delete;
    Profiler(Profiler &&) = delete;
    Profiler &operator=(Profiler &&) = delete;
};