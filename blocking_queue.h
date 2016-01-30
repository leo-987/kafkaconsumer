#ifndef _BLOCKING_QUEUE_H_
#define _BLOCKING_QUEUE_H_

#include <mutex>
#include <condition_variable>
#include <deque>

template <typename T>
class BlockingQueue {
public:
    void Push(T const& value);
    T Pop();

private:
    std::mutex              mutex_;
    std::condition_variable condition_;
    std::deque<T>           queue_;
};

template<typename T>
void BlockingQueue<T>::Push(T const& value)
{
	{
		std::unique_lock<std::mutex> lock(this->mutex_);
		queue_.push_front(value);
	}
	this->condition_.notify_one();
}

template<typename T>
T BlockingQueue<T>::Pop()
{
	std::unique_lock<std::mutex> lock(this->mutex_);
	this->condition_.wait_for(lock, std::chrono::milliseconds(100),
							  [=]{ return !this->queue_.empty(); });
	if (this->queue_.empty())
		return NULL;

	T obj(std::move(this->queue_.back()));
	this->queue_.pop_back();
	return obj;
}

#endif
