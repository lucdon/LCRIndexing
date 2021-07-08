#pragma once

#include <functional>
#include <forward_list>
#include <deque>
#include <condition_variable>
#include <mutex>
#include <atomic>

class WaitHandle : public std::enable_shared_from_this<WaitHandle> {
private:
    std::atomic<uint32_t> counter;
    std::mutex mutex;
    std::condition_variable conditionVariable;
    std::vector<WaitHandle*> toSignalOnFinish;

public:
    explicit WaitHandle(uint32_t count);
    explicit WaitHandle(std::shared_ptr<WaitHandle> &left, std::shared_ptr<WaitHandle> &right);
    ~WaitHandle() = default;

    /**
     * @brief Wait until the work group is completed.
     */
    void waitTillCompleted();

    /**
     * @brief Query if the work group has completed.
     * @return True if the work group has completed.
     */
    bool isCompleted();

private:
    void notifyOnFinished(WaitHandle *toNotify);
    void signalFinished();
    friend class ThreadWorkGroup;
};

class ThreadWorkGroup {
private:
    std::deque<std::function<void(uint32_t id)>> queue;
    std::mutex queueMutex;
    bool closed = false;
    std::shared_ptr<WaitHandle> waitHandle;

public:
    explicit ThreadWorkGroup(uint32_t count) {
        waitHandle = std::make_shared<WaitHandle>(count);
    }

    /**
     * @brief Add a function to the work group
     * @param func the function to add.
     */
    void queueWork(const std::function<void(uint32_t id)> &func);

private:
    friend class ThreadPool;

    /**
     * @brief Get the next function to execute.
     * @param func The next function to execute.
     * @param index The index of the function.
     * @return True if there is new work to execute.
     */
    bool nextWork(std::function<void(uint32_t id)> &func);

    /**
     * @brief Signal that the work has been completed.
     * @param index The index of the completed function.
     */
    void signalFinished();
};
