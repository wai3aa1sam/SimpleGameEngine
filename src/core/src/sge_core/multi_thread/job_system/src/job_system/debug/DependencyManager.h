#pragma once
#include "../base/job_system_base.h"
#include "../job/Job.h"

namespace sge {

class DependencyManager : public NonCopyable
{
	friend class Job;
	friend class JobSystem;
	friend class DepScopeCapture;
public:

	static constexpr size_t s_kLocalBufSize = 4;
	static constexpr size_t s_kMaxFrameCount = 16;

	struct JobInfo
	{
		using DepList = Vector<const Job*, s_kLocalBufSize>;

		DepList dependents;
		DepList runAfterThis;

		const Job* job = nullptr;
		Atomic<size_t> startOrder  = 0;
		Atomic<size_t> finishOrder = 0;
		Atomic<size_t> finishCount = 0;

		JobInfo() : startOrder(0), finishOrder(0), finishCount(0) {}

		void operator=(const JobInfo& rhs)
		{
			dependents		= eastl::move(rhs.dependents);
			runAfterThis	= eastl::move(rhs.runAfterThis);
			job = rhs.job;
			startOrder.store(rhs.startOrder);
			finishOrder.store(rhs.finishOrder);
			finishCount.store(rhs.finishCount);
		}

		void operator=(JobInfo&& rhs) noexcept
		{
			dependents		= eastl::move(rhs.dependents);
			runAfterThis	= eastl::move(rhs.runAfterThis);
			job = rhs.job;
			startOrder.store(rhs.startOrder);
			finishOrder.store(rhs.finishOrder);
			finishCount.store(rhs.finishCount);

			rhs.job = nullptr;
		}
	};
	
	using JobInfoTable = Map<const Job*, JobInfo>;

	struct DependencyInfo
	{
		MutexProtected<JobInfoTable> table;
		MutexProtected<Vector<JobInfo*>> infos;
		Atomic<size_t> nextStartOrder  = 1;
		Atomic<size_t> nextFinishOrder = 1;

		DependencyInfo()
		{
			nextStartOrder = 1;
			nextFinishOrder = 1;
		}

		DependencyInfo(DependencyInfo&& rhs) noexcept
		{
			SGE_ASSERT("shd not be called");
		}
	};
	using FrameDepInfoList		= Vector<DependencyInfo, s_kLocalBufSize>;
	using FrameUnrecordedList	= Vector<Vector<const Job*>, s_kLocalBufSize>;

	using DepInfoStack			= Vector<DependencyInfo, s_kLocalBufSize>;

public:

	DependencyManager();
	~DependencyManager();

	static void printRunAfter();
	static void printRunBefore();

	static void beginCapture();
	static void endCapture(bool verbose = false);

	static void nextFrame();

	static bool checkValid();

protected:
	static DependencyManager* instance() { return _instance; }

	static void addVertex(const Job* job);

	template<class... JOB> static void XRunAfterY(const Job* x, JOB&&... y);
	template<class... JOB> static void XRunBeforeY(const Job* x, JOB&&... y);

	static void jobExecute(const Job* job);
	static void jobFinish(const Job* job);


private:
	DependencyInfo& depInfo();
	void resetFrame();
	
	void _sort(Vector<JobInfo*>& infos);
	void _print_impl(const JobInfoTable& depTable, intptr_t depsMemOffset);
	void _print_info(Vector<JobInfo*>& infos);

	JobInfo* _find(JobInfoTable& table, const Job* job, bool verbose = true);
	JobInfo* _createIfNotExist(JobInfoTable& table, const Job* job, bool verbose = true); // std::integral_constant<bool>

	bool _checkValid(const JobInfoTable& depTable, const Vector<JobInfo*>& infos);

private:
	static DependencyManager* _instance;

	FrameDepInfoList	_depInfoList;
	FrameUnrecordedList _unrecord;
	size_t				_currentFrame = 0;
	size_t				_currentIndex = 0;

	Atomic<int>			_beginCount = 0;

	DepInfoStack		_depInfoStack;
};

inline DependencyManager* DependencyManager::_instance = nullptr;

class DepScopeCapture
{
public:
	static DepScopeCapture ctor(bool verbose = false) { return DepScopeCapture(verbose); }

private:
	DepScopeCapture(bool verbose)	{ _verbose = verbose; DependencyManager::beginCapture(); }
	~DepScopeCapture()				{ DependencyManager::endCapture(_verbose); }
	bool _verbose = false;
};


#if 0
-pragma mark --- DependencyManager-Impl ---
#endif // 0
#if 1

inline DependencyManager::DependencyManager()
{
	SGE_ASSERT(!_instance, "DependencyManager is singleton!");
	_instance = this;

	_depInfoList.resize(s_kMaxFrameCount);
	_unrecord.resize(s_kMaxFrameCount);

	_depInfoStack.reserve(s_kMaxFrameCount);
}

inline DependencyManager::~DependencyManager()
{
	SGE_ASSERT(_instance);

	_instance = nullptr;
}

#if SGE_JOB_SYSTEM_DEBUG

inline void DependencyManager::printRunAfter()
{
	SGE_LOG("=== Dependency Graph: X RunAfter Y");

	auto* dm = instance();
	auto& depInfo = dm->depInfo();
	{
		auto lock_deps = depInfo.table.scopedLock();
		auto& depTable = *lock_deps.operator->();

		dm->_print_impl(depTable, memberOffset(&JobInfo::dependents));
	}
	{
		auto lock_infos = depInfo.infos.scopedLock();
		auto& infos = *lock_infos.operator->();

		dm->_print_info(infos);
	}

	{
		auto lock_deps = depInfo.table.scopedLock();
		auto& depTable = *lock_deps.operator->();

		auto lock_infos = depInfo.infos.scopedLock();
		auto& infos = *lock_infos.operator->();

		if (!dm->_checkValid(depTable, infos))
		{
			throw SGE_ERROR("wrong job dependency");
		}
	}
}

inline void DependencyManager::printRunBefore()
{
	SGE_LOG("=== Dependency Graph: X RunBefore Y");

	auto* dm = instance();
	auto& depInfo = dm->depInfo();

	{
		auto lock_deps = depInfo.table.scopedLock();
		auto& depTable = *lock_deps.operator->();

		dm->_print_impl(depTable, memberOffset(&JobInfo::runAfterThis));
	}
	{
		auto lock_infos = depInfo.infos.scopedLock();
		auto& infos = *lock_infos.operator->();

		dm->_print_info(infos);
	}

	{
		auto lock_deps = depInfo.table.scopedLock();
		auto& depTable = *lock_deps.operator->();

		auto lock_infos = depInfo.infos.scopedLock();
		auto& infos = *lock_infos.operator->();

		if (!dm->_checkValid(depTable, infos))
		{
			throw SGE_ERROR("wrong job dependency");
		}
	}
}

inline void DependencyManager::nextFrame()
{
	auto* dm = instance();
	dm->_currentFrame++;
	if (dm->_currentFrame >= s_kMaxFrameCount)
	{
		dm->_currentIndex = dm->_currentFrame % s_kMaxFrameCount;
	}
	if (dm->_beginCount.load() > 0)
	{
		throw SGE_ERROR("forgot to call endCapture() ?");
	}
	dm->resetFrame();
}

inline bool DependencyManager::checkValid()
{
	auto* dm = instance();
	{
		auto& depInfo = dm->depInfo();
		auto lock_deps = depInfo.table.scopedLock();
		auto& depTable = *lock_deps.operator->();

		auto lock_infos = depInfo.infos.scopedLock();
		auto& infos = *lock_infos.operator->();

		bool isValid = dm->_checkValid(depTable, infos);
		if (!isValid)
		{
			throw SGE_ERROR("wrong job dependency");
		}
		return isValid;
	}
}

inline void DependencyManager::beginCapture()
{
	auto* dm = instance();
	auto& depInfo = dm->_depInfoStack.emplace_back(); (void)depInfo;
	dm->_beginCount.fetch_add(1);

	SGE_LOG("=== DependencyManager::begin()");
}

inline void DependencyManager::endCapture(bool verbose)
{
	SGE_ASSERT(instance()->_depInfoStack.size() > 0);

	auto* dm = instance();
	auto& depInfo = dm->_depInfoStack.back();

	if (verbose)
	{
		SGE_LOG("=== DependencyManager::end(), Graph: X RunAfter Y");
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();

			dm->_print_impl(depTable, memberOffset(&JobInfo::dependents));
		}

		{
			auto lock_infos = depInfo.infos.scopedLock();
			auto& infos = *lock_infos.operator->();

			dm->_print_info(infos);
		}
	}

	{
		auto lock_deps = depInfo.table.scopedLock();
		auto& depTable = *lock_deps.operator->();

		auto lock_infos = depInfo.infos.scopedLock();
		auto& infos = *lock_infos.operator->();

		if (!dm->_checkValid(depTable, infos))
		{
			throw SGE_ERROR("wrong job dependency");
		}
	}

	dm->_depInfoStack.pop_back();
	dm->_beginCount.fetch_sub(1);
	SGE_LOG("=== DependencyManager end scope");

}


#else

inline void DependencyManager::printRunAfter()				{}
inline void DependencyManager::printRunBefore()				{}
inline void DependencyManager::beginCapture()				{}
inline void DependencyManager::endCapture(bool verbose)		{}
inline void DependencyManager::nextFrame()					{}
inline bool DependencyManager::checkValid()					{ return true; }


#endif // #if SGE_JOB_SYSTEM_DEBUG

inline void DependencyManager::addVertex(const Job* job)
{
	auto* dm = instance();

#if 0
	JobInfo* info = nullptr;

	{
		auto lock_deps = dm->depInfo().table.scopedLock();
		auto& depTable = *lock_deps.operator->();

		auto dep_pair = depTable.find(job);
		if (dep_pair != depTable.end())
		{
			SGE_LOG("job: {} already exist", job->name());
			return;
		}
		depTable[job];
		depTable[job].job = job;
		info = &depTable[job];
	}

	{
		auto lock_job = dm->depInfo().infos.scopedLock();
		lock_job->emplace_back(info);
	}
#endif // 0

	{
		auto& depInfo = dm->depInfo();
		auto lock_depTable = depInfo.table.scopedLock();
		auto& depTable = *lock_depTable.operator->();
		JobInfo* info = dm->_createIfNotExist(depTable, job); (void)info;
		{
			auto lock_job = depInfo.infos.scopedLock();
			lock_job->emplace_back(info);
		}
	}

	if (dm->_depInfoStack.size() > 0)
	{
		auto& depInfo = dm->_depInfoStack.back();
		auto lock_depTable = depInfo.table.scopedLock();
		auto& depTable = *lock_depTable.operator->();
		JobInfo* info = dm->_createIfNotExist(depTable, job); (void)info;
		{
			auto lock_job = depInfo.infos.scopedLock();
			lock_job->emplace_back(info);
		}
	}
}

template<class... JOB> inline 
void DependencyManager::XRunAfterY(const Job* x, JOB&&... y)
{
	auto* dm = instance();

#if 0
	auto lock_deps = dm->depInfo().table.scopedLock();
	auto& depTable = *lock_deps.operator->();

	auto dep_pair = depTable.find(x);
	if (dep_pair == depTable.end())
	{
		SGE_LOG("job: {} is not exit", x->name());
		return;
	}

	//auto& target_dep	  = dep_pair->first;
	auto& target_job_info = dep_pair->second;

	(target_job_info.dependents.emplace_back(eastl::forward<JOB>(y)), ...);
	//(dm->XRunBeforeY(eastl::forward<JOB>(y), x), ...);
#endif // 0

	{
		auto& depInfo = dm->depInfo();
		JobInfo* info = nullptr;
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();
			info = dm->_find(depTable, x, false);
		}
		if (!info)
			return;
		(info->dependents.emplace_back(eastl::forward<JOB>(y)), ...);
	}

	if (dm->_depInfoStack.size() > 0)
	{
		auto& depInfo = dm->_depInfoStack.back();
		JobInfo* info = nullptr;
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();
			info = dm->_find(depTable, x, false);
		}
		if (!info)
			return;
		(info->dependents.emplace_back(eastl::forward<JOB>(y)), ...);
	}
}

template<class... JOB> inline 
void DependencyManager::XRunBeforeY(const Job* x, JOB&&... y)
{
	auto* dm = instance();

#if 0
	{
		auto lock_deps = dm->depInfo().table.scopedLock();
		auto& depTable = *lock_deps.operator->();

		auto dep_pair = depTable.find(x);
		if (dep_pair == depTable.end())
		{
			SGE_LOG("job: {} is not exit", x->name());
			return;
		}

		//auto& target_dep	  = dep_pair->first;
		auto& target_job_info = dep_pair->second;

		(target_job_info.runAfterThis.emplace_back(eastl::forward<JOB>(y)), ...);
	}
#endif // 0

	{
		auto& depInfo = dm->depInfo();
		JobInfo* info = nullptr;
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();
			info = dm->_find(depTable, x, false);
		}
		if (!info)
			return;
		(info->runAfterThis.emplace_back(eastl::forward<JOB>(y)), ...);
	}
	
	if (dm->_depInfoStack.size() > 0)
	{
		auto& depInfo = dm->_depInfoStack.back();
		JobInfo* info = nullptr;
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();
			info = dm->_find(depTable, x, false);
		}
		if (!info)
			return;
		(info->runAfterThis.emplace_back(eastl::forward<JOB>(y)), ...);
	}
}

inline DependencyManager::DependencyInfo& DependencyManager::depInfo()
{
	SGE_ASSERT(_currentIndex >= 0 && _currentIndex < s_kMaxFrameCount);
	return _depInfoList[_currentIndex];
}

inline void DependencyManager::resetFrame()
{
	auto* dm = instance();
	auto& depInfo = dm->depInfo();
	depInfo.infos.scopedLock()->clear();
	depInfo.table.scopedLock()->clear();
	depInfo.nextFinishOrder.store(1);
	depInfo.nextStartOrder.store(1);
}

inline void DependencyManager::jobExecute(const Job* job)
{
	auto* dm = instance();
#if 0
	auto lock_deps = dm->depInfo().table.scopedLock();
	auto& depTable = *lock_deps.operator->();

	auto dep_pair = depTable.find(job);
	if (dep_pair == depTable.end())
	{
		//SGE_LOG("job: {} is not exit", x->name());
		return;
	}

	//auto& target_dep	  = dep_pair->first;
	auto& target_job_info = dep_pair->second;

	target_job_info.startOrder.store(dm->depInfo().nextStartOrder);
	dm->depInfo().nextStartOrder++;
#endif // 0

	{
		auto& depInfo = dm->depInfo();
		JobInfo* info = nullptr;
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();
			info = dm->_find(depTable, job, false);
		}
		if (!info)
			return;
		info->startOrder.store(depInfo.nextStartOrder);
		depInfo.nextStartOrder++;
	}

	if (dm->_depInfoStack.size() > 0)
	{
		auto& depInfo = dm->_depInfoStack.back();
		JobInfo* info = nullptr;
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();
			info = dm->_find(depTable, job, false);
		}
		if (!info)
			return;
		info->startOrder.store(depInfo.nextStartOrder);
		depInfo.nextStartOrder++;
	}
}

inline void DependencyManager::jobFinish(const Job* job)
{
	auto* dm = instance();
#if 0
	auto lock_deps = dm->depInfo().table.scopedLock();
	auto& depTable = *lock_deps.operator->();

	auto dep_pair = depTable.find(job);
	if (dep_pair == depTable.end())
	{
		//SGE_LOG("job: {} is not exit", x->name());
		return;
	}

	//auto& target_dep	  = dep_pair->first;
	auto& target_job_info = dep_pair->second;

	target_job_info.finishCount++;
	if (target_job_info.finishOrder.load() == 0) // only set once
	{
		target_job_info.finishOrder.store(dm->depInfo().nextFinishOrder);
	}
	dm->depInfo().nextFinishOrder++;
#endif // 0

	{
		auto& depInfo = dm->depInfo();
		JobInfo* info = nullptr;
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();
			info = dm->_find(depTable, job, false);
		}
		if (!info)
			return;

		info->finishCount++;
		if (info->finishOrder.load() == 0) // only set once
		{
			info->finishOrder.store(depInfo.nextFinishOrder);
		}
		depInfo.nextFinishOrder++;
	}

	if (dm->_depInfoStack.size() > 0)
	{
		auto& depInfo = dm->_depInfoStack.back();
		JobInfo* info = nullptr;
		{
			auto lock_depTable = depInfo.table.scopedLock();
			auto& depTable = *lock_depTable.operator->();
			info = dm->_find(depTable, job, false);
		}
		if (!info)
			return;

		info->finishCount++;
		if (info->finishOrder.load() == 0) // only set once
		{
			info->finishOrder.store(depInfo.nextFinishOrder);
		}
		depInfo.nextFinishOrder++;
	}
}

inline void DependencyManager::_sort(Vector<JobInfo*>& infos)
{
	eastl::quick_sort(infos.begin(), infos.end(), 
		[](JobInfo* lhs, JobInfo* rhs) {
			return lhs->finishOrder < rhs->finishOrder;
		});
}

inline bool DependencyManager::_checkValid(const Map<const Job*, JobInfo>& depTable, const Vector<JobInfo*>& infos)
{
	for (auto& info : infos)
	{
		if (!info->startOrder.load() || !info->finishOrder.load())
			continue;

		for (auto& dep : info->runAfterThis)
		{
			SGE_ASSERT(depTable.find(dep) != depTable.end());
			
			auto& depInfo = depTable.find(dep)->second;
			if (!(info->startOrder.load() < depInfo.startOrder.load() && info->finishOrder.load() < depInfo.finishOrder.load()))
				return false;
		}

		for (auto& dep : info->dependents)
		{
			SGE_ASSERT(depTable.find(dep) != depTable.end());
			auto& depInfo = depTable.find(dep)->second;
			if (!(info->startOrder.load() > depInfo.startOrder.load() && info->finishOrder.load() > depInfo.finishOrder.load()))
				return false;
		}
	}

	return true;
}

inline void DependencyManager::_print_impl(const JobInfoTable& depTable, intptr_t depsMemOffset)
{
	TempString buf;

	for (auto& pair: depTable)
	{
		buf.clear();

		auto& deps = *reinterpret_cast<const JobInfo::DepList*>(reinterpret_cast<const u8*>(&pair.second) + depsMemOffset);
		size_t idx = 0;
		for (auto& dep : deps)
		{
			buf += dep->name();
			if (idx < deps.size() - 1)
				buf += ", ";
			else
				buf += "";
			idx++;
		}
		SGE_LOG("{}: {}", pair.first->name(), buf);
	}
}

inline void DependencyManager::_print_info(Vector<JobInfo*>& infos)
{
	SGE_LOG("=== Job Info");
	auto* dm = instance();

	dm->_sort(infos);
	for (auto& jobInfo : infos)
	{
		SGE_LOG("FinishOrder: {}, Job: {}, Start Order: {}, Finish Count: {}", jobInfo->finishOrder, jobInfo->job->name(), jobInfo->startOrder, jobInfo->finishCount);
	}
}

inline DependencyManager::JobInfo* DependencyManager::_find(JobInfoTable& table, const Job* job, bool verbose)
{
	auto& depTable = table;

	auto dep_pair = depTable.find(job);
	if (dep_pair == depTable.end())
	{
		if (verbose)
		{
			SGE_LOG("job: {} is not exist", job->name());
		}
		return nullptr;
	}

	return &dep_pair->second;
}

inline DependencyManager::JobInfo* DependencyManager::_createIfNotExist(JobInfoTable& table, const Job* job, bool verbose)
{
	JobInfo* info = nullptr;
	
	{
		auto& depTable = table;

		auto dep_pair = depTable.find(job);
		if (dep_pair != depTable.end())
		{
			if (verbose)
			{
				SGE_LOG("job: {} is already exist", job->name());
			}
			return nullptr;
		}
		depTable[job].job = job;
		info = &depTable[job];
	}
	return info;
}

#endif // 1


}