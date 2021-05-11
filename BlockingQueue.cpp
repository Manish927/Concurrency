#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>



template <typename T>
class BlockingQueue
{
	std::queue<T> Q;
	std::mutex mutex;
	std::condition_variable cv;
public:
	BlockingQueue() {}
	BlockingQueue(BlockingQueue<T>&& other)
	{
		std::lock_guard lock(mutex);

		Q = other.Q;
		while (other.Q.size() > 0)
			other.Q.clear();
	}

	BlockingQueue<T>& operator= (BlockingQueue<T>&& other)
	{
		if (this == &other)
			return *this;

		std::lock_guard lock(mutex);
		Q = other.Q;

		while (other.Q.size() > 0)
			other.Q.pop();

		return *this;
	}

	BlockingQueue(const BlockingQueue<T>&) = delete;
	BlockingQueue<T>& operator= (const BlockingQueue<T>&) = delete;

	T deQueue()
	{
		std::unique_lock lock(mutex);

		if (!Q.empty())
		{
			T temp = Q.front();
			Q.pop();
			return temp;
		}

		while (Q.empty())
		{
			cv.wait(lock, [this]() {return (!Q.empty()); });

			T temp = Q.front();
			Q.pop();

			return temp;
		}
	}

	void enQueue(const T& t)
	{
		{
			std::unique_lock<std::mutex> lock(mutex);
			Q.push(t);
		}

		cv.notify_all();
	}

	T& front()
	{
		std::lock_guard lock(mutex);
		T temp;

		if (!Q.empty())
		{
			temp = Q.front();
		}
		else
		{
			cv.wait(lock, [this]() {return (!Q.empty()); });
			temp = Q.front();
		}

		return temp;
	}
	void clear()
	{
		std::lock_guard lock(mutex);

		while (!Q.empty())
			Q.pop();
	}

	size_t size()
	{
		std::lock_guard lock(mutex);
		return Q.size();
	}
};
