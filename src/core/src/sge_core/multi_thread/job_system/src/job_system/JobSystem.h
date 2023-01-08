#pragma once
#include "base/job_system_base.h"

#include "thread/ThreadPool.h"
#include "job/Job.h"

#include "feature/ParallelFor.h"

#include "debug/DependencyManager.h"

namespace sge {

// TODO: prioirty

using JobHandle = Job*;

class JobSystem
{
	friend class WorkerThread;
	friend class ThreadPool;
	friend class _parallel_for_impl;

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

	void clearJobs();

	template<class T>
	JobHandle dispatch(T& obj, u32 loopCount, u32 batchSize, JobHandle dependOn = nullptr);

	JobHandle dispatch(const Task& task, u32 loopCount, u32 batchSize, JobHandle dependOn = nullptr);

	void waitForComplete(JobHandle job);

	JobHandle createJob(const Task& task, void* param);
	
	JobHandle createSubJob(JobHandle parent, const Task& task, void* param);

	JobHandle createEmptyJob();

	template<class T, class BATCHER = ByteBatcher<>>
	JobHandle parallel_for(T* data, size_t count, ParForTask<T> task, BATCHER batcher = ByteBatcher<>());

	size_t		workersCount();
	const char*	threadName();

protected:
	bool _tryGetJob(Job*& job);

	JobHandle _dispatch(const Task& task, u32 loopCount, u32 batchSize, JobHandle dependOn = nullptr);

	void _checkError();

	static u32  dispatchBatchGroup(u32 loopCount, u32 batchSize);

	template<class ALLOCATOR = JobAllocator>
	static Job* allocateJob(ALLOCATOR& allocator = JobSystem::_defaultJobAllocator());

	static void _execute(Job* job);
	static void _complete(Job* job);

	static JobAllocator& _defaultJobAllocator();

private:
	static JobSystem* _instance;

	Vector<UPtr<ThreadStorage>> _storages;

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

template<class T> inline
JobHandle JobSystem::dispatch(T& obj, u32 loopCount, u32 batchSize, JobHandle dependOn)
{
	return _dispatch(std::bind(&T::execute, &obj, std::placeholders::_1), loopCount, batchSize, dependOn);
}

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


#endif


}