#pragma once
#include "../base/job_system_base.h"

#include "../job/JobAllocator.h"

namespace sge {

class ThreadStorage
{
	friend class JobSystem;
public:
	ThreadStorage()
	{
	}

	~ThreadStorage()
	{
		clear();
	}

	i16 localId() const { return _localId; }
	
	Job* allocateJob()	{ return _jobAllocator.allocate(); }
	void clearJobs()	{ return _jobAllocator.clear(); }

	//void* allocateParForData(size_t nBytes) { return _parForAllocator.allocate(nBytes); }
	//void clearParForData()					{ return _parForAllocator.clear(); }

	void clear()
	{
		clearJobs();
		//clearParForData();
	}

private:
	void _init(i16 localId)
	{
		_setLocalId(localId);
	}

	void _setLocalId(i16 localId) { _localId = localId; }

private:
	i16 _localId = -1;
	JobAllocator _jobAllocator;
	//LinearAllocator _parForAllocator;
};



}