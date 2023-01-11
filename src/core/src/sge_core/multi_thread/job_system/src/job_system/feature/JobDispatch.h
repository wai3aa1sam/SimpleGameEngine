#pragma once

#include "../base/job_system_base.h"

namespace sge{

template<class T> class Job_Base;			/* void execute(); */
template<class T> class JobFor_Base;		/* void execute(const JobArgs& args); */
template<class T> class JobParFor_Base;		/* void execute(const JobArgs& args); */ 

template<class T, class ENABLE = void>
struct JobDispatcher
{
	JobDispatcher()
	{
		static_assert(IsBaseOf<Job_Base, T>,		"T is not base of Job_Base");		// TYPE 0
		static_assert(IsBaseOf<JobFor_Base, T>,		"T is not base of JobFor_Base");	// TYPE 1
		static_assert(IsBaseOf<JobParFor_Base, T>,	"T is not base of JobParFor_Base");	// TYPE 2
	}

	template<size_t TYPE, bool IS_DELAY, class T, class... DEPEND_ON>
	static JobHandle _dispatch(T& obj, u32 loopCount, JobHandle dependOn = nullptr, DEPEND_ON&&... moreDeps)
	{
		static_assert(IsBaseOf<Job_Base<T>, T> || IsBaseOf<JobFor_Base<T>, T>, "T is not base of Job_Base or JobFor_Base");

		if (loopCount == 0)
			return nullptr;

		T* pObj = &obj;

		JobFunction task;
		if constexpr (TYPE == Job_Base<T>::s_kType)
		{
			SGE_ASSERT(loopCount == 1);
			static_check<Job_Base<T>>();
			task = [pObj](const JobArgs& args) { return pObj->execute(); };
		}
		else if constexpr (TYPE == JobFor_Base<T>::s_kType)
		{
			static_check<JobFor_Base<T>>();
			task = [pObj](const JobArgs& args) { return pObj->execute(args); };
		}

		Job* job = JobSystem::allocateJob();
		{
			JobInfo info;
			info.batchID	 = 0;
			info.batchOffset = 0 * 0;
			info.batchEnd	 = loopCount;

			job->init(task, info, nullptr);
		}

		if constexpr (!IS_DELAY)
		{
			JobSystem::submit(job);
		}
		else
		{
			if (dependOn)
			{
				job->runAfter(dependOn);
			}
			(job->runAfter(std::forward<DEPEND_ON>(moreDeps)), ...);
		}

		return job;
	}

	template<class K>
	static void static_check()
	{
		//TypeDisplayer<BASE> a;
		if		constexpr (IsSame<Job_Base<T>,		 K>)	{ static_assert(IsBaseOf<Job_Base<T>,		T>,	"T is not base of Job_Base"); }
		else if constexpr (IsSame<JobFor_Base<T>,	 K>)	{ static_assert(IsBaseOf<JobFor_Base<T>,	T>,	"T is not base of JobFor_Base"); }
		else if constexpr (IsSame<JobParFor_Base<T>, K>)	{ static_assert(IsBaseOf<JobParFor_Base<T>,	T>,	"T is not base of JobParFor_Base"); }
		else
		{
			static_assert(false,	"T is not base of Job_Base or JobFor_Base or JobParFor_Base");
		}
	}
};

template<class T>
class Job_Base			/*: public NonCopyable */
{ 
private:
	template<class T, class ENABLE> friend struct JobDispatcher;
	static constexpr size_t s_kType = 0;
	static constexpr const char* s_checkTypeMsg = "T is not base of Job_Base";
	using This = Job_Base<T>;

public:
	/* void execute(); */ 

	JobHandle dispatch()
	{
		return dispatch(*static_cast<T*>(this));
	}

	template<class... DEPEND_ON>
	JobHandle delayDispatch(JobHandle dependOn = nullptr, DEPEND_ON&&... moreDeps)
	{
		return delayDispatch(*static_cast<T*>(this), dependOn, std::forward<DEPEND_ON>(moreDeps)...);
	}

public:
	template<class T>
	static JobHandle dispatch(T& obj)
	{
		return JobDispatcher<T>::_dispatch<s_kType, false>(obj, 1, nullptr);
	}
	//struct JsonIO <SE, T, std::enable_if_t<std::is_enum_v<T>> > {

	// may conflict with delayDispatch if user call delayDispatch(Job*, Job*, Job*);
	template<class T, class ENABLE = EnableIf<!IsSame<T, JobHandle>>, class... DEPEND_ON>
	static JobHandle delayDispatch(T& obj, JobHandle dependOn = nullptr, DEPEND_ON&&... moreDeps)
	{
		return JobDispatcher<T>::_dispatch<s_kType, true>(obj, 1, dependOn, std::forward<DEPEND_ON>(moreDeps)...);
	}
};

template<class T>
class JobFor_Base		/*: public NonCopyable */
{ 
private:
	template<class T, class ENABLE> friend struct JobDispatcher;
	static constexpr size_t s_kType = 1;
	static constexpr const char* s_checkTypeMsg = "T is not base of JobFor_Base";
	using This = JobFor_Base<T>;

public:
	/* void execute(const JobArgs& args); */ 

	JobHandle dispatch(u32 loopCount)
	{
		return dispatch(*static_cast<T*>(this), loopCount);
	}

	template<class... DEPEND_ON>
	JobHandle delayDispatch(u32 loopCount, JobHandle dependOn = nullptr, DEPEND_ON&&... moreDeps)
	{
		return delayDispatch(*static_cast<T*>(this), loopCount, dependOn, std::forward<DEPEND_ON>(moreDeps)...);
	}

public:
	template<class T>
	static JobHandle dispatch(T& obj, u32 loopCount)
	{
		return JobDispatcher<T>::_dispatch<s_kType, false>(obj, loopCount, nullptr);
	}

	template<class T, class... DEPEND_ON>
	static JobHandle delayDispatch(T& obj, u32 loopCount, JobHandle dependOn = nullptr, DEPEND_ON&&... moreDeps)
	{
		return JobDispatcher<T>::_dispatch<s_kType, true>(obj, loopCount, dependOn, std::forward<DEPEND_ON>(moreDeps)...);
	}
};

template<class T>
class JobParFor_Base	/*: public NonCopyable */
{ 
private:
	template<class T, class ENABLE> friend struct JobDispatcher;
	static constexpr size_t s_kType = 2;
	static constexpr const char* s_checkTypeMsg = "T is not base of JobParFor_Base";
	using This = JobParFor_Base<T>;

public:
	/* void execute(const JobArgs& args); */ 

	JobHandle dispatch(u32 loopCount, u32 batchSize)
	{
		return dispatch(*static_cast<T*>(this), loopCount, batchSize);
	}

	template<class... DEPEND_ON>
	JobHandle delayDispatch(u32 loopCount, u32 batchSize, DEPEND_ON&&... dependOn)
	{
		return delayDispatch(*static_cast<T*>(this), loopCount, batchSize, std::forward<DEPEND_ON>(dependOn)...);
	}

public:
	template<class T>
	static JobHandle dispatch(T& obj, u32 loopCount, u32 batchSize)
	{
		JobDispatcher<T>::static_check<This>();
		if (loopCount == 0 || batchSize == 0)
			return nullptr;

		const u32 nBatchGroup	= Math::divideTo(loopCount, batchSize);
		//T* pObj = static_cast<T*>(this);
		T* pObj = &obj;

		JobFunction task = [pObj](const JobArgs& args) { return pObj->execute(args); };

		JobInfo info;
		Job* parent = nullptr;

		for (u32 iBatchGroup = 0; iBatchGroup < nBatchGroup; iBatchGroup++)
		{
			Job* job = JobSystem::allocateJob();
			
			info.batchID	 = iBatchGroup;
			info.batchOffset = iBatchGroup * batchSize;
			info.batchEnd	 = Math::min(info.batchOffset + batchSize, loopCount);

			job->init(task, info, parent);

			if (iBatchGroup == 0)
				parent = job;

			JobSystem::submit(job);
		}
		return parent;
	}

	template<class T,class... DEPEND_ON>
	static JobHandle delayDispatch(T& obj, u32 loopCount, u32 batchSize, DEPEND_ON&&... dependOn)
	{
		JobDispatcher<T>::static_check<This>();
		if (loopCount == 0 || batchSize == 0)
			return nullptr;

		//T* pObj = static_cast<T*>(this);
		T* pObj = &obj;
		Job* spwanJob 	= JobSystem::allocateJob();
		
		auto spwanTask = [spwanJob, pObj, loopCount, batchSize](const JobArgs& args)
		{
			const u32 nBatchGroup	= Math::divideTo(loopCount, batchSize);
			JobFunction task = [pObj](const JobArgs& args) { return pObj->execute(args); };

			JobInfo info;

			for (u32 iBatchGroup = 0; iBatchGroup < nBatchGroup; iBatchGroup++)
			{
				Job* job = JobSystem::allocateJob();

				info.batchID	 = iBatchGroup;
				info.batchOffset = iBatchGroup * batchSize;
				info.batchEnd	 = Math::min(info.batchOffset + batchSize, loopCount);

				job->init(task, info, spwanJob);

				JobSystem::submit(job);
			}
		};

		{
			JobInfo info;
			info.batchEnd = 1;
			spwanJob->init(spwanTask, info);
		}

		(spwanJob->runAfter(std::forward<DEPEND_ON>(dependOn)), ...);

		return spwanJob;
	}
};

}