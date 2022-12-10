#include <sge_core-pch.h>
#include "ThreadBase.h"

#include "Synchronize.h"

namespace sge {

ThreadBase::~ThreadBase()
{ 
	SGE_ASSERT(!_handle, "must call join() on derived class, also should be awaked");
}

void ThreadBase::setThreadAffinity(int k_th_bit)
{
#if SGE_TEST_BASIC_THREAD
	SGE_ASSERT(_handle);
	::SetThreadAffinityMask(_handle, 1LL << k_th_bit);

	//atomicLog("set affinity {}", k_th_bit);
#else

#endif

}

void ThreadBase::join()
{
#if SGE_TEST_BASIC_THREAD
	if (_handle)
	{
		::WaitForSingleObject(_handle, INFINITE);
		this->_handle = 0;

		//atomicLog("thread {} ended", localId());
	}
#else
	_handle.join();
#endif // SGE_TEST_BASIC_THREAD
	
}

}
