#pragma once
#include "../base/job_system_base.h"

#include "WorkerThread.h"
#include "ThreadStorage.h"

namespace sge {

class WorkerThread;
class Job;

struct ThreadPool_CreateDesc
{
	int threadCount = 0;
	int startIndex = 0;
	Vector<UPtr<ThreadStorage>>* threadStorages = nullptr;
};

class ThreadPool : public NonCopyable
{
	friend class WorkerThread;
	using CreateDesc = ThreadPool_CreateDesc;
public:
	ThreadPool() = default;
	ThreadPool(const CreateDesc& desc);
	~ThreadPool();

	void create(const CreateDesc& desc);

	void terminate();

	void submit(Job* job);
	void run();

	bool tryGetJob(Job*& job)
	{
		for (int i = 0; i < _workers.size(); i++)
		{
			if (_workers[i]->queue().try_steal(job))
			{
				return true;
			}
		}
		return false;
	}

	size_t workerCount() const;

protected:
	bool trySteal(WorkerThread* worker, Job*& job);

	int getNextIndex(int i);

private:
	Vector<UPtr<WorkerThread>> _workers;

	PrioityQueue<Job*> _queues;

	Atomic<int> _nextIndex = 0;
	Atomic<bool> _isDone = false;

	int _nfsNextIndex = 0;
	Random _rnd;

	int _startIndex = -1;
};

}


