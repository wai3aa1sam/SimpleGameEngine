#pragma once
#include "../base/job_system_base.h"


#pragma warning( push )
#pragma warning( disable : 4324 )

namespace sge {

class alignas(s_kCacheLine) Job //: public NonCopyable
{
public:
	using Priority = JobPrioity;

	//using Task = Function<void(void*)>;
	using Task =  void(*)(void*);
	
	static constexpr Task s_emptyTask = [](void*) {};

private:
	friend class WorkerThread;
	friend class JobSystem;
	friend class _parallel_for_impl;

	static constexpr size_t s_kTargetSize = s_kCacheLine * 2;

#if 0
#pragma mark --- Storage-Impl ---
#endif // 0
#if 1

	template<size_t N>
	struct LocalBuffer
	{
		static constexpr size_t s_kCapacity = N - 1;
		LocalBuffer()
		{
			static_assert(N <= 255); // max of u8
		}
		void* allocate(size_t n)
		{
			SGE_ASSERT(_offset + n <= s_kCapacity);
			if (_offset + n > s_kCapacity)
				return nullptr;
			auto* ret = _data + _offset;
			_offset += static_cast<u8>(n);
			return ret;
		}
		void clear() { _offset = 0; }

		u8 _data[s_kCapacity];
		u8 _offset = 0;
	};
	template<>
	struct LocalBuffer<0>
	{
	};

	struct NormalData
	{
		NormalData()
		:
			_jobRemainCount(1), _priority(Priority::Cirtical)
		{
		}
		Task				_task;
		void*				_param = nullptr;

		// concept of parent and DepData::runAfterThis is a little bit different.
		// parent may run before the child but DepData::runAfterThis must run after this job
		Job*				_parent = nullptr;		
		Atomic<int>			_jobRemainCount = 1;

		Priority			_priority;

		LocalBuffer<34+1>	_localBuf;
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
		void runAfterThis_for_each_ifNoDeps(FUNC func)
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

		//bool couldRun() const { return _dependencyCount == 0; }
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
			static_assert(sizeof(Data) % s_kCacheLine == 0);
		}
	};
#endif // 1
public:

	~Job() = default;
	
	void setParent(Job* parent);

	bool isCompleted() const;
	int jobRemainCount() const;

	template<class... JOB> void runAfter (JOB&&... job);
	template<class... JOB> void runBefore(JOB&&... job);

	int dependencyCount()  const;
	size_t runAfterCount() const;

	void print() const;

	// only useful when enable SGE_JOB_SYSTEM_DEBUG
	Job* setName(const char* name);
	const char* name() const;

protected:
	void _runAfter(Job* job);
	void _runBefore(Job* job);
	void* _allocate(size_t n);

private:
	void init(Task func, void* param, Job* parent = nullptr);
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