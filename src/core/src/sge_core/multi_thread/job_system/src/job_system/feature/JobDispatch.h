#pragma once

#include "../base/job_system_base.h"

namespace sge{

class Job_Base;			/* void execute(); */
class JobFor_Base;		/* void execute(const JobArgs& args); */
class JobParFor_Base;	/* void execute(const JobArgs& args); */ 

class Job_Base			/*: public NonCopyable */
{ 
	/* void execute(); */					

public:
	template<class T>
	static JobHandle dispatch(T& obj)
	{
		return _dispatch<false>(obj);
	}

	template<class T, class... DEPEND_ON>
	static JobHandle delayDispatch(T& obj, DEPEND_ON&&... dependOn)
	{
		return _dispatch<true>(obj, std::forward<DEPEND_ON>(dependOn)...);
	}

private:
	template<bool IS_DELAY, class T, class... DEPEND_ON>
	static JobHandle _dispatch(T& obj, DEPEND_ON&&... dependOn)
	{
		auto* pObj = &obj;
		JobFunction task = [pObj](const JobArgs& args) { return pObj->execute(); };

		JobInfo info;

		Job* job = JobSystem::allocateJob();
		{
			info.batchID	 = 0;
			info.batchOffset = 0 * 0;
			info.batchEnd	 = 1;
			job->init(task, info, nullptr);
		}
		
		if constexpr (!IS_DELAY)
		{
			JobSystem::submit(job);
		}

		if constexpr (sizeof...(T) > 0)
		{
			(job->runAfter(std::forward<DEPEND_ON>(dependOn)), ...);
		}

		return job;
	}

};

class JobFor_Base		/*: public NonCopyable */
{ 
	/* void execute(const JobArgs& args); */ 

public:
	template<class T>
	static JobHandle dispatch(T& obj, u32 loopCount)
	{
		return _dispatch<false>(obj, loopCount);
	}

	template<class T, class... DEPEND_ON>
	static JobHandle delayDispatch(T& obj, u32 loopCount, DEPEND_ON&&... dependOn)
	{
		return _dispatch<true>(obj, loopCount, std::forward<DEPEND_ON>(dependOn)...);
	}

private:
	template<bool IS_DELAY, class T, class... DEPEND_ON>
	static JobHandle _dispatch(T& obj, u32 loopCount, DEPEND_ON&&... dependOn)
	{
		if (loopCount == 0)
			return nullptr;

		auto* pObj = &obj;
		JobFunction task = [pObj](const JobArgs& args) { return pObj->execute(args); };

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

		if constexpr (sizeof...(T) > 0)
		{
			(job->runAfter(std::forward<DEPEND_ON>(dependOn)), ...);
		}

		return job;
	}
};

class JobParFor_Base	/*: public NonCopyable */
{ 
	/* void execute(const JobArgs& args); */ 

public:

	template<class T>
	static JobHandle dispatch(T& obj, u32 loopCount, u32 batchSize)
	{
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
		if (loopCount == 0 || batchSize == 0)
			return nullptr;

		T* pObj = this;
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

template<class T, class ENABLE = void>
struct JobDispatcher
{
	JobDispatcher()
	{
		static_assert(IsBaseOf<Job_Base, T>,		"T is not base of Job_Base");
		static_assert(IsBaseOf<JobFor_Base, T>,		"T is not base of JobFor_Base");
		static_assert(IsBaseOf<JobParFor_Base, T>,	"T is not base of JobParFor_Base");
	}
};

#if 0

template<class T>
struct JobDispatcher<T, EnableIf<IsBaseOf<Job_Base, T> > >
{
	static JobHandle dispatch(T& obj)
	{
		return _dispatch<false>(obj);
	}

	template<class... DEPEND_ON>
	static JobHandle delayDispatch(T& obj, DEPEND_ON&&... dependOn)
	{
		return _dispatch<true>(obj, std::forward<DEPEND_ON>(dependOn)...);
	}

private:
	template<bool IS_DELAY, class... DEPEND_ON>
	static JobHandle _dispatch(T& obj, DEPEND_ON&&... dependOn)
	{
		auto* pObj = &obj;
		JobFunction task = [pObj](const JobArgs& args) { return pObj->execute(); };

		JobInfo info;

		Job* job = JobSystem::allocateJob();
		job->init(task, nullptr);

		info.batchID	 = 0;
		info.batchOffset = 0 * 0;
		info.batchEnd	 = 1;

		job->_setInfo(info);

		if constexpr (!IS_DELAY)
		{
			JobSystem::submit(job);
		}

		if constexpr (sizeof...(T) > 0)
		{
			(job->runAfter(std::forward<DEPEND_ON>(dependOn)), ...);
		}

		return job;
	}
};

template<class T>
struct JobDispatcher<T, EnableIf<IsBaseOf<JobFor_Base, T> > >
{
	static JobHandle dispatch(T& obj, u32 loopCount)
	{
		return _dispatch<false>(obj, loopCount);
	}

	template<class... DEPEND_ON>
	static JobHandle delayDispatch(T& obj, u32 loopCount, DEPEND_ON&&... dependOn)
	{
		return _dispatch<true>(obj, loopCount, std::forward<DEPEND_ON>(dependOn)...);
	}

private:
	template<bool IS_DELAY, class... DEPEND_ON>
	static JobHandle _dispatch(T& obj, u32 loopCount, DEPEND_ON&&... dependOn)
	{
		if (loopCount == 0)
			return nullptr;

		auto* pObj = &obj;
		JobFunction task = [pObj](const JobArgs& args) { return pObj->execute(args); };

		JobInfo info;

		Job* job = JobSystem::allocateJob();
		job->init(task, nullptr);

		info.batchID	 = 0;
		info.batchOffset = 0 * 0;
		info.batchEnd	 = loopCount;

		job->_setInfo(info);

		if constexpr (!IS_DELAY)
		{
			JobSystem::submit(job);
		}

		if constexpr (sizeof...(T) > 0)
		{
			(job->runAfter(std::forward<DEPEND_ON>(dependOn)), ...);
		}

		return job;
	}
};

template<class T>
struct JobDispatcher<T, EnableIf<IsBaseOf<JobParFor_Base, T> > >
{
	static JobHandle dispatch(T& obj, u32 loopCount, u32 batchSize)
	{
		if (loopCount == 0 || batchSize == 0)
			return nullptr;

		const u32 nBatchGroup	= Math::divideTo(loopCount, batchSize);
		auto* pObj = &obj;
		JobFunction task = [pObj](const JobArgs& args) { return pObj->execute(args); };

		JobInfo info;
		Job* parent = nullptr;

		for (u32 iBatchGroup = 0; iBatchGroup < nBatchGroup; iBatchGroup++)
		{
			Job* job = JobSystem::allocateJob();
			job->init(task, parent);
			if (iBatchGroup == 0)
				parent = job;

			info.batchID	 = iBatchGroup;
			info.batchOffset = iBatchGroup * batchSize;
			info.batchEnd	 = Math::min(info.batchOffset + batchSize, loopCount);

			job->_setInfo(info);

			JobSystem::submit(job);
		}
		return parent;
	}

	template<class... DEPEND_ON>
	static JobHandle delayDispatch(T& obj, u32 loopCount, u32 batchSize, DEPEND_ON&&... dependOn)
	{
		if (loopCount == 0 || batchSize == 0)
			return nullptr;

		T* pObj = &obj;
		Job* spwanJob 	= JobSystem::allocateJob();
		{
			JobInfo info;
			info.batchEnd = 1;
			spwanJob->_setInfo(info);
		}

		auto spwanTask = [spwanJob, pObj, loopCount, batchSize](const JobArgs& args)
		{
			const u32 nBatchGroup	= Math::divideTo(loopCount, batchSize);
			JobFunction task = [pObj](const JobArgs& args) { return pObj->execute(args); };

			JobInfo info;

			for (u32 iBatchGroup = 0; iBatchGroup < nBatchGroup; iBatchGroup++)
			{
				Job* job = JobSystem::allocateJob();
				job->init(task, spwanJob);

				info.batchID	 = iBatchGroup;
				info.batchOffset = iBatchGroup * batchSize;
				info.batchEnd	 = Math::min(info.batchOffset + batchSize, loopCount);

				job->_setInfo(info);

				JobSystem::submit(job);
			}
		};
		spwanJob->init(spwanTask);

		(spwanJob->runAfter(std::forward<DEPEND_ON>(dependOn)), ...);

		return spwanJob;
	}
};

#endif // 0


}