#include "ThreadPool.h"

#include <iostream>

// Constructor implementation
ThreadPool::ThreadPool(size_t numThreads) : stop(false)
{
    // Loop to create 'numThreads' worker threads
    for (size_t i = 0; i < numThreads; ++i)
    {
        // emplace_back constructs a std::thread directly in the vector.
        // The lambda function defines what each worker thread will execute.
        workers.emplace_back([this] { // '[this]' captures the current ThreadPool object by pointer.
            // This loop represents the endless work cycle of a worker thread.
            while (true)
            {
                // Delare a task variable to hold the retrieved task
                std::function<void()> task;

                // ----------------------------------------------
                // Steps:
                // Aquire lock to stop other threads from accessing 'tasks' queue and 'stop' flag (concurrent access)
                // Set condition for thread to wait until the stop flag is true or the tasks queue is not empty to start work after being notified
                // If pool is stopping and no more tasks, exit
                // Get next task
                // Execute task
                // ----------------------------------------------

                // Start a new scope to control the lifetime of the unique_lock.
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);

                    // The 'wait' function atomically releases the lock and blocks the thread.
                    // When notified, it reacquires the lock before returning.
                    this->condition.wait(lock, [this]
                                         { return this->stop || !this->tasks.empty(); });

                    // Check if pool is stopping and tasks are empty, if so, exit
                    // guarentees all tasks are completed
                    if (this->stop && this->tasks.empty())
                        return;

                    // Retrieve the next task from the front of the queue.
                    task = std::move(this->tasks.front()); // Use std::move for efficiency.
                    this->tasks.pop();                     // Remove the task from the queue.

                    // The lock is automatically released here.
                }

                // Execute the task. This happens outside the mutex lock
                // to allow other threads to access the queue while this thread computes.
                try
                {
                    task();
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Thread pool task error: " << e.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "Thread pool task unknown error" << std::endl;
                }
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        // get lock to modify the stop flag
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }

    // notify all worker threads that the pool is stopping
    condition.notify_all();

    // Join all worker threads.
    // 'join()' ensures that the main thread waits for each worker thread to finish
    // its current task (if any) and exit its loop before the destructor completes.
    // This prevents "dangling threads" which could try to access destroyed resources.
    for (std::thread &worker : workers)
    {
        worker.join();
    }
}
