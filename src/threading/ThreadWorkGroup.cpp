#include "threading/ThreadWorkGroup.hpp"
#include "threading/ThreadPool.hpp"
#include <iostream>

void ThreadWorkGroup::queueWork(const std::function<void(uint32_t id)> &func) {
    if (closed) {
        return;
    }

    this->queue.emplace_back(func);
}

bool ThreadWorkGroup::nextWork(std::function<void(uint32_t id)> &func) {
    std::lock_guard<std::mutex> lock(this->queueMutex);

    if (this->queue.empty()) {
        return false;
    }

    func = this->queue.front();
    this->queue.pop_front();
    return true;
}

void ThreadWorkGroup::signalFinished() {
    waitHandle->signalFinished();
}

void WaitHandle::waitTillCompleted() {
    std::unique_lock<std::mutex> lock(mutex);
    conditionVariable.wait(lock, [this]() { return counter == 0; });
}

bool WaitHandle::isCompleted() {
    return this->counter == 0;
}

WaitHandle::WaitHandle(uint32_t count) : counter(count) {
}

WaitHandle::WaitHandle(std::shared_ptr<WaitHandle> &left, std::shared_ptr<WaitHandle> &right) : counter(2) {
    left->notifyOnFinished(this);
    right->notifyOnFinished(this);
}

void WaitHandle::signalFinished() {
    if (this->counter-- == 1) {
        this->conditionVariable.notify_all();

        auto &threadPool = getThreadPool();
        auto thisPtr = shared_from_this();

        threadPool.signalFinished(thisPtr);

        for (auto &toNotify : toSignalOnFinish) {
            toNotify->signalFinished();
        }

        toSignalOnFinish.clear();
    }
}

void WaitHandle::notifyOnFinished(WaitHandle *toNotify) {
    std::lock_guard<std::mutex> lk(this->mutex);

    if (this->isCompleted()) {
        toNotify->signalFinished();
    } else {
        this->toSignalOnFinish.emplace_back(toNotify);
    }
}
