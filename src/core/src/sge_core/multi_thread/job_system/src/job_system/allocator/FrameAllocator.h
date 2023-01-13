#pragma once

#include "JobAllocator.h"

namespace sge {


template<size_t FRAME_COUNT = s_kJobSystemAllocatorFrameCount>
class FrameAllocator_Storage
{
public:
	static constexpr size_t s_kFrameCount = s_kJobSystemAllocatorFrameCount;
public:
	FrameAllocator_Storage()
	{
		for (size_t i = 0; i < s_kFrameCount; i++)
		{
			//_jobAllocators->alloc
		}
	}

	~FrameAllocator_Storage() { clearAll(); }

	void clearAll()
	{
		for (size_t i = 0; i < s_kFrameCount; i++)
		{
			_jobAllocators[i].clear();
			_jobDependencyAllocators[i].clear();
		}
	}

	void nextFrame() 
	{
		iFrame = (iFrame + 1) %  s_kFrameCount;
		clear();
	}
	
	Job* allocJob()							{ return jobAllocator().alloc(); }

	JobAllocator& jobAllocator()			{ return _jobAllocators[iFrame]; }
	JobAllocator& jobDependencyAllocator()	{ return _jobDependencyAllocators[iFrame]; }

protected:
	void clear()
	{
		jobAllocator().clear();
	}

private:
	int iFrame = 0;
	JobAllocator _jobAllocators[FRAME_COUNT];
	JobAllocator _jobDependencyAllocators[FRAME_COUNT];
};

class FrameAllocator : public FrameAllocator_Storage<s_kJobSystemAllocatorFrameCount>
{
public:
	static constexpr size_t s_kFrameCount = SGE_JOB_SYSTEM_ALLOCATOR_FRAME_COUNT;
	using Storage = FrameAllocator_Storage<s_kJobSystemAllocatorFrameCount>;


private:
};

#if 0
#pragma mark --- FrameAllocator_Storage-Impl
#endif // 0
#if 1

#endif

#if 0
#pragma mark --- FrameAllocator-Impl
#endif // 0
#if 1

#endif

}