#pragma once
#include "Job.h"
#include "../JobSystem.h"

namespace sge {

class JobFlow
{
public:
	template<class... JOB>
	auto emplace(JOB&&... job)
	{
		//static_assert(std::is_same<JOB, Job>());
		return std::make_tuple(_emplace(eastl::forward<JOB>(job))...);
	}

	void runAndWait()
	{
		run();
		for (auto& job : _waitForjobs)
		{
			JobSystem::waitForComplete(job);
		}
	}

	void run()
	{
		// only submit job that has no dependency
		for (auto& job : _jobs)
		{
			if (!job->runAfterCount())
			{
				_waitForjobs.emplace_back(job);
			}

			if (!job->dependencyCount())
				JobSystem::submit(job);
		}
	}

private:
	Job* _emplace(Job* job)
	{
		return _jobs.emplace_back(job);
	}

private:
	Vector<Job*, 12> _jobs;
	Vector<Job*, 12> _waitForjobs;
};

//auto a = sizeof(JobFlow);

}