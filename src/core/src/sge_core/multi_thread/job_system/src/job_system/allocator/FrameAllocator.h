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
		std::unique_lock lock(_mtx);
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
		std::unique_lock lock(_mtx);

		auto str = Fmt("current frame thread: {}, frame: {}", threadLocalId(), _iFrame);
		SGE_PROFILE_LOG(str.c_str());
		
		//_iFrame.store((_iFrame.load() + 1) % s_kFrameCount);
		_iFrame = (_iFrame + 1) % s_kFrameCount;

		str.clear();
		FmtTo(str, "current frame thread: {}, frame: {}", threadLocalId(), _iFrame);
		SGE_PROFILE_LOG(str.c_str());
		
		// race condition
		clear();
	}
	
	Job* allocJob()							{ SLock lock(_mtx); return _jobAllocators[_iFrame].alloc(); }

	JobAllocator& jobAllocator()			{ SLock lock(_mtx); return _jobAllocators[_iFrame]; }
	JobAllocator& jobDependencyAllocator()	{ SLock lock(_mtx); return _jobDependencyAllocators[_iFrame]; }

protected:
	void clear()
	{
		// don't lock
		_jobAllocators[_iFrame].clear();
		_jobDependencyAllocators[_iFrame].clear();
	}

private:
	SMutex _mtx;
	int				_iFrame = 0;
	JobAllocator	_jobAllocators[FRAME_COUNT];
	JobAllocator	_jobDependencyAllocators[FRAME_COUNT];
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