#pragma once
#include "../base/job_system_base.h"

#include "../utility/Function.h"

#pragma warning( push )
#pragma warning( disable : 4324 )

namespace sge {

struct JobArgs
{
	using T = u32;

	T loopIndex		= 0;		// SV_DispatchThreadID
	T batchID		= 0;		// SV_GroupID
	T batchIndex	= 0;		// SV_GroupIndex
};

struct JobInfo
{
	using T = u32;
	T batchID		= 0;
	T batchOffset	= 0;
	T batchEnd		= 0;

	void clear() { batchID = 0; batchOffset = 0; batchEnd = 0; }
};

//using JobFunction = Function<void(JobArgs&), 32>;
using JobFunction = std::function<void(JobArgs&)>;

class alignas(s_kCacheLine) Job //: public NonCopyable
{
	SGE_JOB_SYSTEM_JOB_TYPE_FRIEND_CLASS_DECLARE();

public:
	using Priority = JobPrioity;
	using Info = JobInfo;

	using  Task = JobFunction;
	static Task s_emptyTask;

	//using Task =  void(*)(void*);
	//static constexpr Task s_emptyTask = [](void*) {};

private:
	friend class WorkerThread;
	friend class JobSystem;
	friend class _parallel_for_impl;

	static constexpr size_t s_kTargetSize = s_kCacheLine * 2;

#if 0
#pragma mark --- Storage-Impl ---
#endif // 0
#if 1

	struct NormalData
	{
		NormalData()
		:
			_jobRemainCount(1), _priority(Priority::Cirtical)
		{
			#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
			_resetDebugCheck();
			#endif // _DEBUG
		}

		#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
		void _resetDebugCheck()
		{
			_isAllowAddDeps.store(true);
			_isExecuted.store(false);
			_isSubmitted.store(false);
		}
		#endif // _DEBUG

		void setPriority(Priority pri)	{ _priority.store(pri); }
		Priority priority()				{ return _priority.load(); }

		Task				_task;
		Info				_info;

		// concept of parent and DepData::runAfterThis is a little bit different.
		// parent may run before the child but DepData::runAfterThis must run after this job
		Job*				_parent = nullptr;		
		Atomic<int>			_jobRemainCount = 1;


		#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
		Atomic<bool>		_isAllowAddDeps = true;
		Atomic<bool>		_isExecuted		= false;
		Atomic<bool>		_isSubmitted	= false;
		#endif // _DEBUG

	private:
		Atomic<Priority>	_priority;
		//Priority			_priority;
	};
	struct DepData
	{
		friend class Job;

		static constexpr size_t s_kSizeWithoutDeps = sizeof(NormalData);
		//static constexpr size_t s_kLocalDepsCount = (s_kTargetSize - s_kSizeWithoutDeps - sizeof(Vector<Job*>)) / sizeof(Job*) - 1;
		static constexpr size_t s_kLocalDepsCount = 2;

		using DepList = Vector<Job*, s_kLocalDepsCount>;

		DepData()
			:
			_dependencyCount(0)
		{
		}

		template<class FUNC>
		void runAfterThis_for_each(FUNC func)
		{
			for (auto& job : _runAfterThis)
			{
				func(job);
			}
		}

		template<class FUNC>
		void runAfterThis_for_each_ifNoDeps(const FUNC& func)
		{
			for (auto& job : _runAfterThis)
			{
				int count = job->decrDependencyCount();
				if (count == 0)
				{
					func(job);
				}
			}
		}

		int	decrDependencyCount()	{ return --_dependencyCount; }

		bool couldRun() const { return _dependencyCount.load() == 0; }
		/*
		Consider: atomic var called counter; a function logic. if counter == 0 then can perform xxx. suppose the xxx only could run once.
		situation: thread A decrement counter then context switch to thread B and thread B decrement counter.
		both of them will do if counter == 0 is true and perform xxx.
		*/

	private:
		Atomic<int>	_dependencyCount = 0;
		DepList		_runAfterThis;
		//DepList		_runBeforeThis;
	};
	struct Data : public NormalData
	{
		DepData dep;
	};

	struct Storage : public Data
	{
		Storage()
		{
			sizeof(Data);
			//static_assert(sizeof(Data) % s_kCacheLine == 0);
		}
	};
#endif // 1
public:

	~Job() = default;
	
	void clear();


	bool isCompleted() const;
	int jobRemainCount() const;

	template<class... JOB> void runAfter (JOB&&... job);
	template<class... JOB> void runBefore(JOB&&... job);

	int dependencyCount()  const;
	size_t runAfterCount() const;

	const Info& info() const;

	void print() const;

	// only useful when enable SGE_JOB_SYSTEM_DEBUG
	Job* setName(const char* name);
	const char* name() const;

protected:
	void setParent(Job* parent);

	void _runAfter(Job* job);
	void _runBefore(Job* job);
	void* _allocate(size_t n);


private:
	void init(const Task& func, const Info& info, Job* parent = nullptr);
	void _setInfo(const Info& info);

	void setEmpty();

	void addJobCount();
	int	decrDependencyCount();

private:
	Storage _storage;

#if SGE_JOB_SYSTEM_DEBUG
	String _name;
#endif // SGE_JOB_SYSTEM_DEBUG

};

template<class... JOB> inline 
void Job::runAfter(JOB&&... job)
{
	(_runAfter(eastl::forward<JOB>(job)), ...);
}

template<class... JOB> inline 
void Job::runBefore(JOB&&... job)
{
	(_runBefore(eastl::forward<JOB>(job)), ...);
}

}


#pragma warning( pop )