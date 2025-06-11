#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <type_traits>

class ThreadPool
{
public:
    // Makes thread pool with specified number of threads
    explicit ThreadPool(size_t numThreads);

    ~ThreadPool();

    // Enqueues a task into the thread pool.
    // Callable: The type of the function/lambda to execute.
    // Args: The types of arguments to pass to the function.
    template <class Callable, class... Args>
    auto enqueue(Callable &&callable, Args &&...args)
        -> std::future<std::invoke_result_t<Callable, Args...>>
    {
        // get return type of the callable using the args
        using return_type = std::invoke_result_t<Callable, Args...>;

        // Create a packaged_task. This object will hold the function and its arguments,
        // and its associated future will eventually hold the result of the function call
        // std::bind is used to bind the function `callable` with its `args`, creating a
        // callable object that takes no arguments (as required by std::packaged_task).
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Callable>(callable), std::forward<Args>(args)...));

        // Get the future associated with this packaged_task.
        // This future will be returned to the caller, allowing them to retrieve the result later.
        std::future<return_type> result = task->get_future();

        {
            // Acquire a unique_lock on the queue_mutex.
            // This ensures that only one thread can modify the 'tasks' queue at a time.
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop)
                throw std::runtime_error("Enqueu on a stopped ThreadPool");

            // Add the task to the queue.
            // The task is wrapped in a lambda so it can be stored as std::function<void()>.
            // When this lambda is called by a worker thread, it will execute the packaged_task,
            // which in turn executes the original function 'callable' and sets the result in its future.
            tasks.emplace([task]()
                          { (*task)(); });
        } // The lock is automatically released here when 'lock' goes out of scope.

        // Notify one waiting worker thread that a new task is available.
        condition.notify_one();
        // Notify one waiting worker thread that a new task is available.
        return result;
    }

private:
    // worker threads
    std::vector<std::thread> workers;
    // task queue
    std::queue<std::function<void()>> tasks;
    // mutex, without this threads could try to pop the same task, or push a task at the same time, leading to data corruption (a "race condition")
    std::mutex queue_mutex;
    // condition variable
    std::condition_variable condition;
    // indicates when thread pool should be stopped
    bool stop;
};
