#include "Job.h"
#include "../debug/DependencyManager.h"

#include "../JobSystem.h"

namespace sge {

Job::Task Job::s_emptyTask = [](const JobArgs& args) {};

void Job::waitForComplete()
{
	JobSystem::instance()->waitForComplete(this);
}

void Job::submit()
{
	JobSystem::submit(this);
}

void Job::clear()
{
	_storage.dep._dependencyCount.store(0);
	_storage.dep._runAfterThis.clear();

	_storage._task = nullptr;
	_storage._info.clear();

	_storage._jobRemainCount.store(1);
	_storage._parent = nullptr;

	_storage.setPriority(Job::Priority::Cirtical);

	#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
	_storage._resetDebugCheck();
	#endif // _DEBUG
}

void Job::setParent(Job* parent)		
{ 
	#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
	SGE_ASSERT(!_storage._isSubmitted);
	SGE_ASSERT(!_storage._isExecuted);
	#endif // 0

	// maybe shd set recursively
	if (!parent)
		return;
	parent->addJobCount(); 
	_storage._parent = parent; 
}

bool Job::isCompleted() const		{ return _storage._jobRemainCount.load() == 0 || _storage._task == nullptr; }
int  Job::jobRemainCount() const	{ return _storage._jobRemainCount.load(); }

void Job::_runAfter(Job* job)
{
	#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
	SGE_ASSERT(job->_storage._isAllowAddDeps);
	SGE_ASSERT(_storage._isAllowAddDeps);
	#endif // 0

	SGE_ASSERT(job != this);
	//SGE_ASSERT(isMainThread(), "must only add job dependency in main thread");

	this->_storage.dep._dependencyCount.fetch_add(1);
	job->_storage.dep._runAfterThis.emplace_back(this);	// _runAfterThis is correct, not worng

	#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::instance()->XRunBeforeY(job, this);
	DependencyManager::instance()->XRunAfterY(this, job);
	#endif // SGE_JOB_SYSTEM_DEBUG
}

void Job::_runBefore(Job* job)
{
	#if SGE_JOB_SYSTEM_IS_CONDITION_DEBUG
	SGE_ASSERT(job->_storage._isAllowAddDeps);
	SGE_ASSERT(_storage._isAllowAddDeps);
	#endif // 0

	SGE_ASSERT(job != this);
	//SGE_ASSERT(isMainThread(), "must only add job dependency in main thread");

	job->_storage.dep._dependencyCount.fetch_add(1);
	this->_storage.dep._runAfterThis.emplace_back(job);	// _runAfterThis is correct, not worng

#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::instance()->XRunBeforeY(this, job);
	DependencyManager::instance()->XRunAfterY(job, this);
#endif // SGE_JOB_SYSTEM_DEBUG
}

void* Job::_allocate(size_t n)
{
	return nullptr /*this->_storage._localBuf.allocate(n)*/;
}

void Job::_setInfo(const Info& info)
{
	_storage._info = info;
}

int		Job::dependencyCount() const	{ return _storage.dep._dependencyCount.load(); }
size_t	Job::runAfterCount() const		{ return _storage.dep._runAfterThis.size(); }

void Job::print() const
{
	//atomicLog("job -> jobRemainCount: {}", _storage._jobRemainCount);
	//atomicLog("job -> dependencyCount: {}", dependencyCount());
}

void Job::init(const Task& func, const Info& info, Job* parent)
{
	clear();

	_storage._jobRemainCount.store(1);
	_storage._task = func;

	_setInfo(info);

	setParent(parent);
}

void Job::addJobCount()				{ _storage._jobRemainCount++; }
int	 Job::decrDependencyCount()		{ return _storage.dep.decrDependencyCount(); }

const Job::Info& Job::info() const
{
	return _storage._info;
}

Job* Job::setName(const char* name) 
{
#if SGE_JOB_SYSTEM_DEBUG
	_name = name;
#endif // SGE_JOB_SYSTEM_DEBUG

	return this;
}

void Job::setEmpty()
{
	init(s_emptyTask, JobInfo(), nullptr);
}

const char* Job::name() const
{
#if SGE_JOB_SYSTEM_DEBUG
	return _name.c_str();
#else
	return nullptr;
#endif // SGE_JOB_SYSTEM_DEBUG
}


}