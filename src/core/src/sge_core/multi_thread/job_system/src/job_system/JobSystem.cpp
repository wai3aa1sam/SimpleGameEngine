#include "JobSystem.h"

#include <sge_core/profiler/sge_profiler.h>

namespace sge {

JobSystem* JobSystem::_instance = nullptr;

JobSystem::JobSystem(int threadTypeCount)
{
	SGE_ASSERT(logicalThreadCount() == s_kJobSystemLogicalThreadCount, "please set marco SGE_JOB_SYSTEM_HARDWARE_THREAD_COUNT to correct number");
	SGE_ASSERT(threadTypeCount <= logicalThreadCount());
	
	_instance = this;
	size_t total_thread = logicalThreadCount();
	_threadTypeCount = threadTypeCount;

	int nWorkers = static_cast<int>(total_thread - _threadTypeCount);
	int start_index = _threadTypeCount;
	SGE_ASSERT(total_thread >= _threadTypeCount);

	//_threadStorages.reserve(total_thread);
	for (size_t i = 0; i < total_thread; i++)
	{
		auto& back = _threadStorages.emplace_back(new ThreadStorage());
		back->_init(static_cast<i16>(i));
	}

	setThreadLocalId(s_kMainThread);
	_threadStorages[s_kMainThread]->_setName("Main Thread");

	if (nWorkers)
	{
		ThreadPool_CreateDesc desc;
		desc.startIndex = start_index;
		desc.threadCount = nWorkers;
		desc.threadStorages = &_threadStorages;
		_threadPool.create(desc);
	}
}

JobSystem::~JobSystem()
{
	SGE_ASSERT(_instance);

	_threadPool.terminate();

	_instance = nullptr;
}

void JobSystem::waitForComplete(Job* job)
{
	auto* jsys = this;
	auto& threadPool = this->_threadPool;
	auto& storage = *jsys->_threadStorages[s_kMainThread]; (void)storage;

	while (!job->isCompleted())
	{
		Job* tmp = nullptr;

		if (threadPool.tryGetJob(tmp))
		{
			_execute(tmp);
		}

		sleep_ms(s_kBusySleepTimeMS);
	}

	//atomicLog("=== done waitForComplete()");
}

void JobSystem::submit(JobHandle job)
{
	#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
	SGE_ASSERT(!job->_storage._isSubmitted);
	SGE_ASSERT(!job->_storage._isExecuted);

	job->_storage._isSubmitted.store(true);
	job->_storage._isAllowAddDeps.store(false);
	#endif // 0

	#if SGE_JOB_SYSTEM_ENABLE_SINGLE_THREAD_DEBUG
	_execute(job);
	#else

	if (!job->_storage.dep.couldRun())
	{
		// rare case, maybe have some bug, all job should sumbit when dep count is 0
		// 19/1/2023: no bug now, since _complete() are not reading and compare the copy, 
		// when contex-switch, possibly call _complete() multiple times which causing calling multipy runAfterThisIffNoDeps(),
		// couldRun() would be compare negative number, since it is executed
		SGE_ASSERT(job->_storage.dep.couldRun());
	}

	auto& threadPool = instance()->_threadPool;
	threadPool.submit(job);
#endif
}

void JobSystem::_internal_nextFrame()
{
	SGE_ASSERT(threadLocalId() == s_kMainThread);

	#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::nextFrame();
	#endif // 0

	for (auto& ts : _threadStorages)
	{
		ts->nextFrame();
	}
}

ThreadStorage& JobSystem::_threadStorage()
{
	auto* jsys = this;
	jsys->_checkError();
	auto* storage = jsys->_threadStorages[threadLocalId()].get();
	return *storage;
}

JobHandle JobSystem::createEmptyJob()
{
	auto* job = JobSystem::allocateJob();
	job->setEmpty();
	return job;
}

JobSystem::SizeType JobSystem::workersCount()  const
{
	auto* jsys = this;
	jsys->_checkError();
	return jsys->_threadPool.workerCount();
}

const char* JobSystem::threadName() const
{
	auto* jsys = this;
	jsys->_checkError();
	return jsys->_threadStorages[threadLocalId()]->name();
}

JobSystem::SizeType	JobSystem::workerStartIdx()	const { return _threadTypeCount; }
JobSystem::SizeType	JobSystem::workersEndIdx()  const { return s_kJobSystemLogicalThreadCount - 1; }

bool JobSystem::_tryGetJob(Job*& job)
{
	return _threadPool.tryGetJob(job);
}

JobAllocator& JobSystem::_defaultJobAllocator()
{
	auto* jsys = JobSystem::instance();
	jsys->_checkError();

	auto* storage = jsys->_threadStorages[threadLocalId()].get();
	return storage->jobAllocator();
}

void JobSystem::_checkError() const
{
	if (!(threadLocalId() >= 0 && threadLocalId() < this->_threadStorages.size()))
	{
		atomicLog("=== threadLocalId() {}, localId {} _checkError()", threadLocalId(), this->_threadStorages[threadLocalId()]->localId());
	}

	SGE_ASSERT(threadLocalId() >= 0 && threadLocalId() < this->_threadStorages.size());
	SGE_ASSERT(this->_threadStorages[threadLocalId()]);
}

void JobSystem::_complete(Job* job)
{
	auto& jobRemainCount	= job->_storage._jobRemainCount;
	auto& parent			= job->_storage._parent;
	//auto& depsOnThis		= job->_storage.dep._depsOnThis;

	//atomicLog("=== task complete");
	int ret = jobRemainCount.fetch_sub(1) - 1;
	// must have a copy, consider the jobRemainCount maybe 1, when contex-switch, 
	// other thread is decr the jobRemainCount, both of them will trigger jobRemainCount == 0,
	// possibly call _complete() multiple times which causing calling multipy runAfterThisIffNoDeps(),
	
	#if SGE_JOB_SYSTEM_DEBUG
	if (!parent)
		DependencyManager::jobFinish(job);
	#endif // 0

	if (ret == 0)	// must compare
	{
		if (parent)
		{
			_complete(parent);
		}

		job->_storage.dep.runAfterThis_for_each_ifNoDeps(JobSystem::submit);
	}
}

void JobSystem::_execute(Job* job)
{
	#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::jobExecute(job);
	#endif // 0

	#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
	SGE_ASSERT(job->_storage._isSubmitted);
	SGE_ASSERT(!job->_storage._isExecuted);

	job->_storage._isAllowAddDeps.store(false);
	job->_storage._isExecuted.store(true);
	#endif // 0

	auto& task  = job->_storage._task;
	auto& info	= job->info();

	JobArgs args;
	args.batchID = info.batchID;

	for (u32 i = info.batchOffset; i < info.batchEnd; ++i)
	{
		args.loopIndex  = i;
		args.batchIndex = i - info.batchOffset;
		task(args);
	}

	_complete(job);
}

}