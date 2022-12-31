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

	i16			localId() const	{ return _localId; }
	const char* name() const	{ return _name.c_str(); }

	Job* allocateJob()	{ return _jobAllocator.allocate(); }
	void clearJobs()	{ return _jobAllocator.clear(); }

	//void* allocateParForData(size_t nBytes) { return _parForAllocator.allocate(nBytes); }
	//void clearParForData()					{ return _parForAllocator.clear(); }

	void clear()
	{
		clearJobs();
		//clearParForData();
	}

	void wake() { resetSleepCount(); }
	void sleep()
	{
		if (shouldSleep())
		{
			sleep_ms(s_kIdleSleepTimeMS);

			//SGE_PROFILE_LOG(Fmt("Thread {} Idle sleep", localId()).c_str());
		}
		else
		{
			sleep_ms(s_kBusySleepTimeMS);
			addSleepCount();

			//SGE_PROFILE_LOG(Fmt("Thread {} Busy sleep", localId()).c_str());
		}
	}

private:
	void _init(i16 localId, StrView name = StrView())
	{
		_setLocalId(localId);
		_setName(name);
	}

	void _setName(StrView name)
	{
		if (name.size() == 0)
		{
			_name = "Thread_";
			_name.append(StringUtil::toString(localId()));
			return;
		}
		_name = name;
	}

	void _setLocalId(i16 localId) { _localId = localId; }

	void resetSleepCount()		{ _sleepCount = 0; }
	void addSleepCount()		{ _sleepCount++; }
	bool shouldSleep() const	{ return _sleepCount >= _sleepThreshold; }
	
private:
	i16 _localId = -1;
	String _name;

	JobAllocator _jobAllocator;
	//LinearAllocator _parForAllocator;
	int _sleepCount		= 0;
	int _sleepThreshold = 2000;
};



}