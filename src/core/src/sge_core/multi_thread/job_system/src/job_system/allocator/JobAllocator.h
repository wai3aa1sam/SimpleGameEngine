#pragma once

#include "../base/job_system_base.h"
#include "temp_LinearAllocator.h"
#include "../job/Job.h"

namespace sge {

class JobAllocator
{
public:
	JobAllocator(int count = 2048)
		:
		_allocator(sizeof(Job)* count)
	{
		SGE_ASSERT(isPowOfTwo(count));
	}

	~JobAllocator()
	{
		clear();
		//clear();
	}

	Job* alloc(u32 size = sizeof(Job))
	{
		Job* job = static_cast<Job*>(_allocator.allocate(size));
		return job;
	}

	void clear()
	{
		_allocator.destructAndClear<Job>();
	}

private:
	temp::LinearAllocator _allocator;
};

}