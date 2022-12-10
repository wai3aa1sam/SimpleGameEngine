#pragma once
#include "../base/job_system_base.h"


#include "ThreadStorage.h"

namespace sge {

class ThreadPool;
class Job;

using JobStealQueue = StealQueue<Job*>;

class WorkerThread : public ThreadBase
{
public:
	WorkerThread(ThreadPool* threadPool, ThreadStorage* storage);
	~WorkerThread();
	void submit(Job* task);

	void onProc();

	void terminate();

	void ready();

	void run();

	JobStealQueue& queue() { return _jobs; }

private:
	bool _tryGetJob(Job*& job);
	
	void _setReady()
	{
#if 0
		{
			auto isReady = _isReady.scopedLock();
			*isReady = true;
		}
		_isReady.notifyAll();
#endif // 0
	}

	template<class... ARGS>
	void debugLog(ARGS&&... args)
	{
		//atomicLog(SGE_FORWARD(args)...);
	}

	template<class... ARGS>
	void log(ARGS&&... args)
	{
		atomicLog(SGE_FORWARD(args)...);
	}

private:
	JobStealQueue _jobs;

	ThreadPool*		_threadPool = nullptr;
	ThreadStorage*	_storage	= nullptr;

#if 0
	ConVarProtected<bool> _isReady;
#endif // 0

};


}