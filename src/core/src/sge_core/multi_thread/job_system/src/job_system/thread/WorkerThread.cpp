#include <sge_core-pch.h>
#include "WorkerThread.h"
#include "../job/Job.h"

#include "ThreadPool.h"

#include "../JobSystem.h"

namespace sge {

thread_local int _threadLocalId = -1;

WorkerThread::WorkerThread(ThreadPool* threadPool, ThreadStorage* storage)
{
	_threadPool = threadPool;
	_storage = storage;
}

WorkerThread::~WorkerThread()
{
	debugLog("thread {} ~WorkerThread()", localId());
	terminate();
}

void WorkerThread::submit(Job* task)
{
	_jobs.push(task);
	_setReady();
	//debugLog("thread {} submit(), current task size: {}", localId(), _jobs.size());
}

void WorkerThread::onProc()
{
	_threadLocalId = _storage->localId();

	std::exception_ptr ptr = nullptr;
	debugLog("=== _threadLocalId {}, localId {} onProc()", _threadLocalId, localId());


	try
	{
		Job* job = nullptr;

		for (;;)
		{
			while (job)
			{
				debugLog("=== thread {} execute job", localId());
				JobSystem::_execute(job);
				//job->_execute();
				job = nullptr;
				_jobs.try_pop(job);
			}

			if (!_tryGetJob(job))
			{
			//	log("=== thread {} end, queue count {}", _threadLocalId, _jobs.size());
				return;
			}
		}
	}
	catch(...)
	{
		ptr = std::current_exception();
	}

}

void WorkerThread::terminate()
{
	_setReady();
	join();
}

void WorkerThread::ready()
{
}

void WorkerThread::run()
{
}

bool WorkerThread::_tryGetJob(Job*& job)
{
	job = nullptr;

	if (_jobs.try_pop(job))
	{
		debugLog("=== thread {} _jobs.try_pop()", _threadLocalId);
		return true;
	}

	if (_threadPool->trySteal(this, job))
	{
		debugLog("=== thread {} trysteal()", _threadLocalId);
		return true;
	}
	
#if 0
	// {1}
	{
		auto isReady = _isReady.scopedLock();
		if (!*isReady) // maybe preempted and call submit/terminate by threadPool in {1}, isReady = true
		{
			*isReady = false;
			while (!*isReady)
			{
				isReady.wait();
			}
		}
	}
#else
	//sleep_ms(100);
	sleep_ms(10);

#endif // 0

	if (_threadPool->_isDone && !_jobs.size())
	{
		return false;
	}

	return true;

}

}
