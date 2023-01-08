#include "Job.h"
#include "../debug/DependencyManager.h"

namespace sge {

Job::Task Job::s_emptyTask = [](const JobArgs& args) {};

void Job::setParent(Job* parent)		
{ 
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
	SGE_ASSERT(!job->_storage._isExecuting);
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
	SGE_ASSERT(!job->_storage._isExecuting);
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

void Job::_setInfo(const Info& info)
{
	_storage._info = info;
}

int		Job::dependencyCount() const	 { return _storage.dep._dependencyCount; }
size_t	Job::runAfterCount() const { return _storage.dep._runAfterThis.size(); }

void Job::print() const
{
	atomicLog("job -> jobRemainCount: {}", _storage._jobRemainCount);
	atomicLog("job -> dependencyCount: {}", dependencyCount());
}

void Job::init(const Task& func, void* param, Job* parent)
{
	_storage._jobRemainCount = 1;
	_storage._task = func;
	_storage._param = param;
	setParent(parent);
}

void Job::init(const Task& func, Job* parent)
{
	_storage._jobRemainCount = 1;
	_storage._task = func;
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
	init(s_emptyTask, nullptr, nullptr);
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