#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <functional>
#include <condition_variable>
#include "threading/ThreadWorkGroup.hpp"

class ThreadPool {
private:
    std::mutex workGroupMutex;
    std::condition_variable threadSleepConditionVariable;
    bool terminated = false;
    uint32_t numThreads;
    std::vector<std::thread> threads;
    std::deque<std::shared_ptr<ThreadWorkGroup>> workGroups = { };
    std::unordered_map<std::shared_ptr<WaitHandle>, std::vector<std::shared_ptr<ThreadWorkGroup>>> workGroupWaitings = { };

public:
    ThreadPool();
    ~ThreadPool();

    /**
     * @brief Queue a function for execution.
     * @param func the function to execute.
     * @return A waitHandle to wait or query when the function is finished.
     */
    std::shared_ptr<WaitHandle> queueWork(const std::function<void(uint32_t id)>& func);

    /**
 * @brief Queue a function for execution.
 * @param func the function to execute.
 * @return A waitHandle to wait or query when the function is finished.
 */
    std::shared_ptr<WaitHandle> queueWork(const std::function<void(uint32_t id)>& func, std::shared_ptr<WaitHandle>& handleToWaitFor);

    /**
     * @brief Queue a list of functions.
     * @param functions the functions to execute.
     * @return A waitHandle to wait or query when the functions are finished.
     */
    std::shared_ptr<WaitHandle> queueWorkGroup(std::vector<std::function<void(uint32_t id)>> &functions);

    /**
     * @brief Queue a list of functions.
     * @param functions the functions to execute.
     * @return A waitHandle to wait or query when the functions are finished.
     */
    std::shared_ptr<WaitHandle> queueWorkGroup(std::vector<std::function<void(uint32_t id)>> &functions, std::shared_ptr<WaitHandle>& handleToWaitFor);

private:
    static void run(ThreadPool* threadPool, uint32_t id);

    void signalFinished(std::shared_ptr<WaitHandle>& waitHandle);

    friend class ThreadWorkGroup;
    friend class WaitHandle;
};

/**
 * Get the globally shared thread pool.
 */
ThreadPool& getThreadPool();
void destroyThreadPool();
