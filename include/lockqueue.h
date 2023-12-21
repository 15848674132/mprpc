#ifndef LOCKQUEUE_H
#define LOCKQUEUE_H
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <iostream>

template<typename T>
class LockQueue {
public:
    void Push(const T& data) {
        std::unique_lock<std::mutex> lock(m_mutex);
        // std::cout << "Push data: " << data << std::endl;
        m_queue.push(data);
        m_cond.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [&]() -> bool { return !m_queue.empty(); });
        // 这里出问题了
        // T& data = m_queue.front();
        T data = m_queue.front();
        m_queue.pop();
        // std::cout << "pop data: " << data << std::endl;
        return data;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};


#endif