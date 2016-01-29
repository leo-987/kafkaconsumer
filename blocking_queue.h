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


template<typename T>
void BlockingQueue<T>::push(T const& value)
{
	{
		std::unique_lock<std::mutex> lock(this->mutex_);
		queue_.push_front(value);
	}
	this->condition_.notify_one();
}

template<typename T>
T BlockingQueue<T>::pop()
{
	std::unique_lock<std::mutex> lock(this->mutex_);
	this->condition_.wait(lock, [=]{ return !this->queue_.empty(); });
	T rc(std::move(this->queue_.back()));
	this->queue_.pop_back();
	return rc;
}

#endif
