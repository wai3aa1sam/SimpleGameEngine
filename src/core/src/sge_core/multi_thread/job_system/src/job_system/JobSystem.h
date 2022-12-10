#pragma once
#include "base/job_system_base.h"

#include "thread/ThreadPool.h"
#include "job/Job.h"

#include "feature/ParallelFor.h"

#include "debug/DependencyManager.h"

namespace sge {

// TODO: prioirty

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

	JobSystem();
	~JobSystem();
	static JobSystem* instance() { return _instance; }

	static void waitForComplete(Job* job);

	static void submit(Job* job);

	static Job* createJob(Task task, void* param);
	static Job* createSubJob(Job* parent, Task task, void* param);
	template<class T> static Job* createAndRunNJobs(Task task, T* continuous_data, size_t n);

	static void clearJobs();

	template<class T, class BATCHER = ByteBatcher<>>
	static Job* parallel_for(T* data, size_t count, ParForTask<T> task, BATCHER batcher = ByteBatcher<>());

	static size_t workersCount();

protected:
	static Job* allocateJob();
	bool _tryGetJob(Job*& job);

	static void _execute(Job* job);
	static void _complete(Job* job);

	void _dependencyManager_complete(Job* job);

	void _checkError();

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

template<class T> inline 
Job* JobSystem::createAndRunNJobs(Task task, T* continuous_data, size_t n)
{
	auto waitJob = createJob(Job::s_emptyTask, nullptr);
	auto* p = continuous_data;
	for (size_t i = 0; i < n; i++, ++p)
	{
		auto* subJob = createSubJob(waitJob, task, p);
		submit(subJob);
	}
	submit(waitJob);
	return waitJob;
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