#include "Job.h"
#include "../debug/DependencyManager.h"

namespace sge {

void Job::setParent(Job* parent)		
{ 
	if (!parent)
		return;
	parent->addJobCount(); 
	_storage._parent = parent; 
}

bool Job::isCompleted() const		{ return _storage._jobRemainCount.load() == 0; }
int  Job::jobRemainCount() const	{ return _storage._jobRemainCount.load(); }

void Job::_runAfter(Job* job)
{
	SGE_ASSERT(job != this);
	this->_storage.dep._dependencyCount.fetch_add(1);
	job->_storage.dep._runAfterThis.emplace_back(this);	// _runAfterThis is correct, not worng

#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::instance()->XRunBeforeY(job, this);
	DependencyManager::instance()->XRunAfterY(this, job);
#endif // SGE_JOB_SYSTEM_DEBUG
}

void Job::_runBefore(Job* job)
{
	SGE_ASSERT(job != this);
	job->_storage.dep._dependencyCount.fetch_add(1);
	this->_storage.dep._runAfterThis.emplace_back(job);	// _runAfterThis is correct, not worng

#if SGE_JOB_SYSTEM_DEBUG
	DependencyManager::instance()->XRunBeforeY(this, job);
	DependencyManager::instance()->XRunAfterY(job, this);
#endif // SGE_JOB_SYSTEM_DEBUG
}

void* Job::_allocate(size_t n)
{
	return this->_storage._localBuf.allocate(n);
}

int		Job::dependencyCount() const	 { return _storage.dep._dependencyCount; }
size_t	Job::runAfterCount() const { return _storage.dep._runAfterThis.size(); }

void Job::print() const
{
	atomicLog("job -> jobRemainCount: {}", _storage._jobRemainCount);
	atomicLog("job -> dependencyCount: {}", dependencyCount());
}

void Job::init(Task func, void* param, Job* parent)
{
	_storage._jobRemainCount = 1;
	_storage._task = func;
	_storage._param = param;
	setParent(parent);
}

void Job::addJobCount()				{ _storage._jobRemainCount++; }
int	 Job::decrDependencyCount()		{ return _storage.dep.decrDependencyCount(); }

Job* Job::setName(const char* name) 
{
#if SGE_JOB_SYSTEM_DEBUG
	_name = name;
#endif // SGE_JOB_SYSTEM_DEBUG

	return this;
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