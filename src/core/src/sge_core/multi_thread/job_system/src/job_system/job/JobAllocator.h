#pragma once
#include "../base/job_system_base.h"
#include "../allocator/temp_LinearAllocator.h"
#include "Job.h"

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
		//clear();
	}

	Job* alloc(u32 size = sizeof(Job))
	{
		Job* job = static_cast<Job*>(_allocator.allocate(size));
		//_jobs.emplace_back(job);
		return job;
	}

	void clear()
	{
		//for (auto* job : _jobs)
		//{
		//	//job->print();
		//	job->~Job();
		//}
		//_allocator.clear();

		_allocator.destructAndClear<Job>();
	}

private:
	temp::LinearAllocator _allocator;
	//Vector<Job*, 64> _jobs; // TODO: remove temp
};

}