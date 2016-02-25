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
    T Front();

private:
    std::mutex              mutex_;
    std::condition_variable condition_;
    std::deque<T>           queue_;
};

template<typename T>
void BlockingQueue<T>::Push(T const& value)
{
	{
		std::unique_lock<std::mutex> lock(mutex_);
		queue_.push_back(value);
	}
	condition_.notify_one();
}

template<typename T>
T BlockingQueue<T>::Pop()
{
	std::unique_lock<std::mutex> lock(mutex_);

	//if (timeout > 0)
	//	condition_.wait_for(lock, std::chrono::milliseconds(timeout),
	//							  [=]{ return !queue_.empty(); });
	//else
	//	condition_.wait(lock, [=]{ return !queue_.empty(); });
	
	condition_.wait(lock, [=]{ return !queue_.empty(); });

	T obj(std::move(queue_.front()));
	queue_.pop_front();
	return obj;
}

template<typename T>
T BlockingQueue<T>::Front()
{
	std::unique_lock<std::mutex> lock(mutex_);

	condition_.wait(lock, [=]{ return !queue_.empty(); });
	if (queue_.empty())
		return NULL;
	else
		return queue_.front();
}

#endif
