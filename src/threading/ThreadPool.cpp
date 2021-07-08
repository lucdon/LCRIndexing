#include "threading/ThreadPool.hpp"
#include <iostream>

void ThreadPool::run(ThreadPool *threadPool, uint32_t id) {
    std::shared_ptr<ThreadWorkGroup> currentWorkGroup = nullptr;
    std::function<void(uint32_t id)> currentFunction;
    std::mutex threadSleepMutex;

    // Keep running as long as the thread pool has not been terminated.
    while (!threadPool->terminated) {
        if (currentWorkGroup == nullptr) {
            // Lock on the work group to prevent multiple deque-ing.
            std::unique_lock<std::mutex> lock(threadPool->workGroupMutex);

            while (!threadPool->workGroups.empty()) {
                // Get the next workgroup
                currentWorkGroup = threadPool->workGroups.front();

                // Check if there is work to be done
                if (currentWorkGroup->queue.empty()) {
                    // pop the group and fetch next.
                    threadPool->workGroups.pop_front();
                    currentWorkGroup = nullptr;
                } else {
                    break;
                }
            }

            // If there is no more work to be done, sleep
            if (threadPool->workGroups.empty()) {
                lock.unlock();

                std::unique_lock<std::mutex> lk(threadSleepMutex);

                // Sleep until terminated or more work is ready.
                threadPool->threadSleepConditionVariable.wait(lk, [threadPool] {
                    return !threadPool->workGroups.empty() || threadPool->terminated;
                });

                continue;
            }
        } else {
            // While there is work, do work.
            while (currentWorkGroup->nextWork(currentFunction)) {
                // Actually execute the work.
                currentFunction(id);

                // Signal that the work has been completed.
                currentWorkGroup->signalFinished();
            }

            currentWorkGroup = nullptr;
        }
    }
}

ThreadPool::ThreadPool() {
    numThreads = std::thread::hardware_concurrency();

    for (uint32_t i = 0u; i < numThreads; i++) {
        std::thread &thread = threads.emplace_back(run, this, i);
    }
}

ThreadPool::~ThreadPool() {
    terminated = true;
    threadSleepConditionVariable.notify_all();

    for (auto &thread : threads) {
        thread.join();
    }
}

std::shared_ptr<WaitHandle> ThreadPool::queueWork(const std::function<void(uint32_t id)> &func) {
    std::unique_lock<std::mutex> lock(workGroupMutex);

    // Create a new work group.
    std::shared_ptr<ThreadWorkGroup> workGroup = std::make_shared<ThreadWorkGroup>(1);
    workGroups.emplace_back(workGroup);
    workGroup->queueWork(func);
    workGroup->closed = true;

    lock.unlock();

    // notify that new work is available.
    this->threadSleepConditionVariable.notify_one();

    return workGroup->waitHandle;
}

std::shared_ptr<WaitHandle>
ThreadPool::queueWork(const std::function<void(uint32_t id)> &func, std::shared_ptr<WaitHandle> &handleToWaitFor) {
    std::shared_ptr<ThreadWorkGroup> workGroup = std::make_shared<ThreadWorkGroup>(1);
    workGroup->queueWork(func);
    workGroup->closed = true;

    if (!handleToWaitFor->isCompleted()) {
        workGroupWaitings[handleToWaitFor].emplace_back(workGroup);
    } else {
        workGroups.emplace_back(workGroup);
    }

    return workGroup->waitHandle;
}

std::shared_ptr<WaitHandle> ThreadPool::queueWorkGroup(std::vector<std::function<void(uint32_t id)>> &functions) {
    std::unique_lock<std::mutex> lock(workGroupMutex);

    // Create a new work group.
    std::shared_ptr<ThreadWorkGroup> workGroup = std::make_shared<ThreadWorkGroup>(uint32_t(functions.size()));
    workGroups.emplace_back(workGroup);

    for (const auto &func : functions) {
        workGroup->queueWork(func);
    }

    workGroup->closed = true;

    lock.unlock();

    // notify that new work is available.
    this->threadSleepConditionVariable.notify_all();

    return workGroup->waitHandle;
}

std::shared_ptr<WaitHandle> ThreadPool::queueWorkGroup(std::vector<std::function<void(uint32_t id)>> &functions,
                                                       std::shared_ptr<WaitHandle> &handleToWaitFor) {
    std::shared_ptr<ThreadWorkGroup> workGroup = std::make_shared<ThreadWorkGroup>(uint32_t(functions.size()));

    for (const auto &func : functions) {
        workGroup->queueWork(func);
    }

    workGroup->closed = true;

    if (!handleToWaitFor->isCompleted()) {
        workGroupWaitings[handleToWaitFor].emplace_back(workGroup);
    } else {
        workGroups.emplace_back(workGroup);
    }

    return workGroup->waitHandle;
}

void ThreadPool::signalFinished(std::shared_ptr<WaitHandle> &waitHandle) {
    std::unique_lock<std::mutex> lock(workGroupMutex);

    if (workGroupWaitings.count(waitHandle) > 0) {
        for (auto &workGroup : workGroupWaitings[waitHandle]) {
            workGroups.emplace_back(workGroup);
        }

        workGroupWaitings.erase(waitHandle);

        lock.unlock();

        // notify that new work is available.
        this->threadSleepConditionVariable.notify_all();
    }
}

static ThreadPool *threadPoolInstance;

ThreadPool &getThreadPool() {
    if (threadPoolInstance == nullptr) {
        threadPoolInstance = new ThreadPool();
    }

    return *threadPoolInstance;
}

void destroyThreadPool() {
    delete threadPoolInstance;
    threadPoolInstance = nullptr;
}



