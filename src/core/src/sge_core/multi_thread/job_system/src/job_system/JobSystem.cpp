#include "JobSystem.h"

#include <sge_core/profiler/sge_profiler.h>

namespace sge {

JobSystem* JobSystem::_instance = nullptr;

JobSystem::JobSystem()
{
	SGE_ASSERT(enumInt(ThreadType::Count) <= hardwareThreadCount());
	_instance = this;

	size_t total_thread = hardwareThreadCount();
	int thread_type_count = enumInt(ThreadType::Count);
	int nWorkers = static_cast<int>(total_thread - thread_type_count);
	int start_index = thread_type_count;
	SGE_ASSERT(total_thread >= thread_type_count);

	//_storages.reserve(total_thread);
	for (size_t i = 0; i < total_thread; i++)
	{
		auto& back = _storages.emplace_back(new ThreadStorage());
		back->_init(static_cast<i16>(i));
	}

	_threadLocalId = enumInt(ThreadType::Main);
	_storages[enumInt(ThreadType::Main)]->_setName("Main Thread");

	if (nWorkers)
	{
		ThreadPool_CreateDesc desc;
		desc.startIndex = start_index;
		desc.threadCount = nWorkers;
		desc.threadStorages = &_storages;
		_threadPool.create(desc);
	}
}

JobSystem::~JobSystem()
{
	SGE_ASSERT(_instance);

	_threadPool.terminate();

	_instance = nullptr;
}

void JobSystem::waitForComplete(JobHandle job)
{
	auto* jsys = this;
	auto& threadPool = this->_threadPool;
	auto& storage = *jsys->_storages[enumInt(ThreadType::Main)]; (void)storage;

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

	auto& threadPool = instance()->_threadPool;
	threadPool.submit(job);
#endif
}

void JobSystem::clearJobs()
{
	auto* jsys = this;
	jsys->_checkError();

	auto* storage = jsys->_storages[threadLocalId()].get();
	storage->clearJobs();
}

#if 0
JobHandle JobSystem::createJob(const Task& task)
{
	auto* job = createSubJob(nullptr, task);

	#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::addVertex(job);
	#endif // 0

	return job;
}

JobHandle JobSystem::createSubJob(JobHandle parent, const Task& task)
{
	auto* job = JobSystem::allocateJob();
	job->init(task, parent);
	return job;
}

#endif // 0

JobHandle JobSystem::createEmptyJob()
{
	auto* job = JobSystem::allocateJob();
	job->setEmpty();
	return job;
}


size_t JobSystem::workersCount() 
{
	auto* jsys = this;
	jsys->_checkError();
	return jsys->_threadPool.workerCount();
}

const char* JobSystem::threadName()
{
	auto* jsys = this;
	jsys->_checkError();
	return jsys->_storages[threadLocalId()]->name();
}

bool JobSystem::_tryGetJob(Job*& job)
{
	return _threadPool.tryGetJob(job);
}

JobAllocator& JobSystem::_defaultJobAllocator()
{
	auto* jsys = JobSystem::instance();
	jsys->_checkError();

	auto* storage = jsys->_storages[threadLocalId()].get();
	return storage->jobAllocator();
}

void JobSystem::_checkError()
{
	if (!(threadLocalId() >= 0 && threadLocalId() < this->_storages.size()))
	{
		atomicLog("=== threadLocalId() {}, localId {} _checkError()", threadLocalId(), this->_storages[threadLocalId()]->localId());
	}

	SGE_ASSERT(threadLocalId() >= 0 && threadLocalId() < this->_storages.size());
	SGE_ASSERT(this->_storages[threadLocalId()]);
}

void JobSystem::_complete(Job* job)
{
	auto& jobRemainCount	= job->_storage._jobRemainCount;
	auto& parent			= job->_storage._parent;
	//auto& depsOnThis		= job->_storage.dep._depsOnThis;

	//atomicLog("=== task complete");
	jobRemainCount--;

	#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::jobFinish(job);
	#endif // 0

	if (jobRemainCount.load() == 0)
	{
		if (parent)
		{
			_complete(parent);

			#if SGE_JOB_SYSTEM_DEBUG
			DependencyManager::jobFinish(parent->_storage._parent);
			#endif // 0
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