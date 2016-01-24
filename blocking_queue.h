#ifndef _BLOCKING_QUEUE_H_
#define _BLOCKING_QUEUE_H_

#include <mutex>
#include <condition_variable>
#include <deque>

template <typename T>
class BlockingQueue {
public:
    void push(T const& value);
    T pop();

private:
    std::mutex              mutex_;
    std::condition_variable condition_;
    std::deque<T>           queue_;
};

#endif
