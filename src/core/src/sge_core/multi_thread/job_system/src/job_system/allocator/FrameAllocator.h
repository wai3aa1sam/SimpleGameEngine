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
		auto iFrame_data = _iFrame.scopedULock();
		auto& iFrame = *iFrame_data; (void)iFrame;

		for (size_t i = 0; i < s_kFrameCount; i++)
		{
			_jobAllocators[i].clear();
			_jobDependencyAllocators[i].clear();
		}
	}

	void nextFrame() 
	{
		//SGE_ASSERT(false, "currently have race condition, when the job is allocating job between frame.");
		// no share lock, then the job may allocating and clear immediately
		auto iFrame_data = _iFrame.scopedULock();
		auto& iFrame = *iFrame_data;

		
		//_iFrame.store((_iFrame.load() + 1) % s_kFrameCount);
		iFrame = (iFrame + 1) % s_kFrameCount;

		// race condition (if no lock)
		clear(iFrame_data);
	}
	
	Job* allocJob()							{ auto iFrame_data = _iFrame.scopedSLock(); auto& iFrame = *iFrame_data; return _jobAllocators[iFrame].alloc(); }

	JobAllocator& jobAllocator()			{ auto iFrame_data = _iFrame.scopedSLock(); auto& iFrame = *iFrame_data; return _jobAllocators[iFrame]; }
	JobAllocator& jobDependencyAllocator()	{ auto iFrame_data = _iFrame.scopedSLock(); auto& iFrame = *iFrame_data; return _jobDependencyAllocators[iFrame]; }

private:
	template<class T>
	void clear(T& iFrame_data)
	{
		auto& iFrame = *iFrame_data;

		// don't lock
		_jobAllocators[iFrame].clear();
		_jobDependencyAllocators[iFrame].clear();
	}

private:
	SMutexProtected<int>	_iFrame;
	JobAllocator			_jobAllocators[FRAME_COUNT];
	JobAllocator			_jobDependencyAllocators[FRAME_COUNT];
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