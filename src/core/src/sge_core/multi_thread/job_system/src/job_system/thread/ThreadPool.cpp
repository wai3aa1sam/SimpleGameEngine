#include "ThreadPool.h"

#include "../JobSystem.h"

namespace sge {

ThreadPool::ThreadPool(const CreateDesc& desc)
{
	create(desc);
}

ThreadPool::~ThreadPool()
{
	atomicLog("~ThreadPool()");
	//terminate();
}

void ThreadPool::create(const CreateDesc& desc)
{
	SGE_ASSERT(desc.threadCount <= hardwareThreadCount());
	size_t thread_count = (desc.threadCount == 0) ? hardwareThreadCount() : desc.threadCount;
	//thread_count = ( hardwareThreadCount() - 1) / 2;
	//thread_count = 1;

	_startIndex = desc.startIndex;

	try
	{
		_workers.reserve(thread_count);
		for (int i = 0; i < thread_count; i++)
		{
			auto localId = desc.startIndex + i;
			auto* pThreadStorages = (desc.threadStorages->data() + localId)->get();

			auto& back = _workers.emplace_back(new WorkerThread(this, pThreadStorages));
			back->startProc(localId, &(*back));
		}
	}
	catch (...)
	{
		atomicLog("ThreadPool create failed");
	}
}

void ThreadPool::terminate()
{
	_isDone = true;

	for (auto& t : _workers)
	{
		//atomicLog("ThreadPool::terminate(), thread {}", t->localId());
		t->terminate();
	}
}

void ThreadPool::submit(Job* job)
{
	//_queues.push(job);

	//atomicLog("ThreadPool::submit(), thread {}", _workers[_nextIndex]->localId());
	_workers[_nextIndex]->submit(job);
	_nextIndex = getNextIndex(_nextIndex);
}

void ThreadPool::run()
{
	/*for (auto& w : this->_workers)
	{
		w->run();
	}*/

	Job* job = nullptr;
	while (_queues.try_pop(job))
	{
		_workers[_nfsNextIndex]->submit(job);
		_nfsNextIndex = getNextIndex(_nfsNextIndex);
	}
}


bool ThreadPool::trySteal(WorkerThread* worker, Job*& job)
{
	JobSystem::instance()->_checkError();
	auto stealAttempt = 0;

	while (stealAttempt < _workers.size())
	{
		auto target = _rnd.get<size_t>(0, _workers.size() - 1);
		(target != _threadLocalId - _startIndex) ? _workers[target]->queue().try_steal(job) : nullptr;
		if (job)
			return true;
		stealAttempt++;
	}
	return false;
}

int ThreadPool::getNextIndex(int i)
{
	i++;
	if (i >= _workers.size() || i < 0)
		i = 0;
	return i;
}

size_t ThreadPool::workerCount() const { return _workers.size(); }

}
