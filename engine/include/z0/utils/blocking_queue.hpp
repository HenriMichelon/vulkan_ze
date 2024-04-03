#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>

namespace z0 {
    template<typename T>
    class BlockingQueue {
    public:
        BlockingQueue(size_t _maxSize) : maxSize{_maxSize} {}

        void push(const T& item) {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this] { return queue.size() < maxSize || _shutdown; });
            if (_shutdown) return;
            queue.push(item);
            lock.unlock();
            cond.notify_one();
        }

        bool pop(T& value) {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this] { return !queue.empty() || _shutdown; });
            if (queue.empty() && _shutdown) return false;
            value = queue.front();
            queue.pop();
            lock.unlock();
            cond.notify_one();
            return true;
        }

        void shutdown() {
            {
                std::lock_guard<std::mutex> lock(mutex);
                _shutdown = true;
            }
            cond.notify_all();
        }

        void waitWhileNotEmpty() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this] { return queue.empty() || _shutdown; });
        }

        bool isEmpty() {
            return queue.empty();
        }

    private:
        std::mutex mutex;
        std::condition_variable cond;
        std::queue<T> queue;
        size_t maxSize;
        bool _shutdown{false};
    };

}