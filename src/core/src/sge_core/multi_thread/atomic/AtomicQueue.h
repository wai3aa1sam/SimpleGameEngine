#pragma once
#include "Synchronize.h"
#include "MutexProtected.h"

namespace sge {

#if 0

template<class T>
class ThreadSafeQueue
{
public:
	struct Node
	{
		UPtr<T>    data;
		UPtr<Node> next;
		Node* prev = nullptr;
	};

public:
	ThreadSafeQueue() = default;
	~ThreadSafeQueue() = default;

	void push(const T& ele)
	{
		auto data = _data.scopedLock();
		auto& head = data->_head;
		auto& tail = data->_tail;
		if (!head->data)
		{
			data.reset(new T);
			tail = head.ptr();
			return;
		}
		else
		{
			auto* newNode = new Node;
			newNode->data.reset(new T);
			newNode->prev = tail;
			tail->next.reset(newNode);

			tail = newNode;
		}
	}

	bool try_pop(T& out)
	{
		auto data = _data.scopedLock();
		auto& head = data->_head;
		auto& tail = data->_tail;
		if (!head->data)
		{
			return false;
		}

		out = std::move(*head->data);
		auto newHead = head->next;
		head.reset(newHead);

		return true;
	}

	void isEmpty() const
	{
		auto data = _data.scopedLock();
		auto& head = data->_head;
		return !head->data;
	}

private:
	MutexProtected<Queue<T>> _queue;

	struct Data
	{
		UPtr<Node> _head;
		Node*		_tail = nullptr;
	};
	MutexProtected<Data> _data;
};

#endif // 0

#if 1

template<class T>
class StealQueue : public NonCopyable
{
public:
	using SizeType = size_t;

	bool isEmpty() const
	{
		auto queue = _queue.scopedLock();
		return queue->size() == 0;
	}

	SizeType size() const
	{
		auto queue = _queue.scopedLock();
		return queue->size();
	}

	void push(const T& data)
	{
		auto queue = _queue.scopedLock();
		queue->push_back(data);
	}

	bool try_pop(T& o)
	{
		auto queue = _queue.scopedLock();

		if (!queue->size())
			return false;
		o = eastl::move(queue->back());
		queue->pop_back();
		return true;
	}

	bool try_steal(T& o)
	{
		auto queue = _queue.scopedLock();

		if (!queue->size())
			return false;
		o = eastl::move(queue->front());
		queue->pop_front();
		return true;
	}

private:
	mutable MutexProtected<Deque<T>> _queue;
};

template<class T, u32 PRIORITY_COUNT>
class PrioityQueues
{
public:
	static constexpr u32 s_kPriorityCount = PRIORITY_COUNT;
	using Priority = u32;

	void push(const T& val, Priority pri = s_kPriorityCount - 1)
	{
		SGE_ASSERT(pri >=0 && pri < s_kMaxPriority,"PrioityQueues<T>: invalid priority");
		_queues[pri].push(val);
	}

	bool try_pop(T& o)
	{
		for (size_t i = 0; i < s_kPriorityCount; i++)
		{
			if (_queues[i].try_pop(o))
				return true;
		}
		return false;
	}

private:
	StealQueue<T> _queues[s_kPriorityCount];
};

#endif // 1



}