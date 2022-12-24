#include "JobSystem.h"


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

#if SGE_JOB_SYSTEM_ENABLE_SINGLE_THREAD_DEBUG

	_threadLocalId = 0;

#else

	_threadLocalId = enumInt(ThreadType::Main);

#endif // SGE_JOB_SYSTEM_SINGLE_THREAD_DEBUG

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

void JobSystem::waitForComplete(Job* job)
{
	auto& threadPool = JobSystem::instance()->_threadPool;

	while (!job->isCompleted())
	{
		Job* tmp = nullptr;

		if (threadPool.tryGetJob(tmp))
		{
			_execute(tmp);
		}
		else
		{
			sleep_ms(10);
		}
	}

	//atomicLog("=== done waitForComplete()");
}

void JobSystem::submit(Job* job)
{

#if SGE_JOB_SYSTEM_ENABLE_SINGLE_THREAD_DEBUG
	_execute(job);
#else
	auto& threadPool = JobSystem::instance()->_threadPool;
	threadPool.submit(job);
#endif
}

Job* JobSystem::allocateJob()
{
	auto* jsys = JobSystem::instance();
	jsys->_checkError();

	auto* storage = jsys->_storages[_threadLocalId].get();
	auto* job = storage->allocateJob();

	return job;
}

Job* JobSystem::createJob(Task task, void* param)
{
	auto* job = allocateJob();
	job->init(task, param);

#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::addVertex(job);
#endif // 0

	return job;
}

Job* JobSystem::createSubJob(Job* parent, Task task, void* param)
{
	auto* job = allocateJob();
	job->init(task, param, parent);
	return job;
}

void JobSystem::clearJobs()
{
	auto* jsys = JobSystem::instance();
	jsys->_checkError();

	auto* storage = jsys->_storages[_threadLocalId].get();
	storage->clearJobs();
}

bool JobSystem::_tryGetJob(Job*& job)
{
	return _threadPool.tryGetJob(job);
}

size_t JobSystem::workersCount() 
{
	auto* jsys = JobSystem::instance();
	jsys->_checkError();
	return jsys->_threadPool.workerCount();
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

	auto& task  = job->_storage._task;
	auto& param = job->_storage._param;
	task(param);
	_complete(job);
}

void JobSystem::_checkError()
{
	if (!(_threadLocalId >= 0 && _threadLocalId < JobSystem::instance()->_storages.size()))
	{
		atomicLog("=== _threadLocalId {}, localId {} _checkError()", _threadLocalId, JobSystem::instance()->_storages[_threadLocalId]->localId());
	}

	SGE_ASSERT(_threadLocalId >= 0 && _threadLocalId < JobSystem::instance()->_storages.size());
	SGE_ASSERT(JobSystem::instance()->_storages[_threadLocalId]);
}

}