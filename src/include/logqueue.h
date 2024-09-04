#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// 异步写日志的队列
template <typename T>
class LogQueue {
public:
    void push(const T &data);
    T pop();

private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _condVar;
};

template <typename T>
void LogQueue<T>::push(const T &data)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(data);
    _condVar.notify_one();
}

template <typename T>
T LogQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(_mutex);
    while (_queue.empty()) {
        _condVar.wait(lock);
    }

    T data = _queue.front();
    _queue.pop();
    return data;
}