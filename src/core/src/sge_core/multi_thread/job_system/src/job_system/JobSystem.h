#pragma once
#include "base/job_system_base.h"

#include "thread/ThreadPool.h"
#include "job/Job.h"

#include "feature/ParallelFor.h"
#include "feature/JobDispatch.h"
#include "debug/DependencyManager.h"

namespace sge {

// TODO: prioirty

class JobSystem
{
	friend class WorkerThread;
	friend class ThreadPool;
	friend class _parallel_for_impl;

	SGE_JOB_SYSTEM_JOB_TYPE_FRIEND_CLASS_DECLARE();

private:
	template <class T> using ParForData = typename _parallel_for_impl::Data<T>;
	template <class T> using ParForTask = typename ParForData<T>::Task;

public:
	using Task = Job::Task;
	using CRef_JobHandle = const JobHandle&;

public:
	static JobSystem* instance() { return _instance; }
public:
	JobSystem();
	~JobSystem();

	static void submit(JobHandle job);
	void waitForComplete(JobHandle job);

	JobHandle createEmptyJob();

	//template<class T, class BATCHER = ByteBatcher<>>
	//JobHandle parallel_for(T* data, size_t count, ParForTask<T> task, BATCHER batcher = ByteBatcher<>());

	size_t		workersCount();
	const char*	threadName();

public:
	void _internal_nextFrame();
	
protected:
	static JobAllocator& _defaultJobAllocator();

	ThreadStorage& _threadStorage();

private:
	bool _tryGetJob(Job*& job);
	void _checkError();

	template<class ALLOCATOR = JobAllocator>
	static Job* allocateJob(ALLOCATOR& allocator = JobSystem::_defaultJobAllocator());

	static void _execute(Job* job);
	static void _complete(Job* job);

private:
	static JobSystem* _instance;

	Vector<UPtr<ThreadStorage>> _threadStorages;

	ThreadPool _threadPool;

#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager _dependencyManager;
#endif // SGE_JOB_SYSTEM_DEBUG

};

#if 0
#pragma mark --- Storage-Impl ---
#endif // 0
#if 1

template<class ALLOCATOR> inline
Job* JobSystem::allocateJob(ALLOCATOR& allocator)
{
	return allocator.alloc(sizeof(Job));
}

#if 0
template<class T, class BATCHER> inline
Job* JobSystem::parallel_for(T* data, size_t count, ParForTask<T> task, BATCHER batcher)
{
	auto* jsys = JobSystem::instance();
	jsys->_checkError();

	static_assert(std::is_array<T>::value == false, "parallel_for<T> is array type, maybe not wanted");
	static_assert(std::is_pointer<std::is_pointer<T>>::value == false, "parallel_for<T> is pointer pointer type, maybe not wanted");

	Job* job = jsys->allocateJob();
	auto* buf = job->_allocate(sizeof(ParForData<T>));
	auto* par_data = new(buf) ParForData<T>(job, data, count, task);

	job->init(_parallel_for_impl::invoke<T, BATCHER>, par_data);

	#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::addVertex(job);
	#endif // 0

	return job;
}
#endif // 0


#endif


}