
#include "blocking_queue.h"

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
