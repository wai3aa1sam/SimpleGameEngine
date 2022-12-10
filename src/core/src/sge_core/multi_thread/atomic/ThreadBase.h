#pragma once

namespace sge {

#define SGE_TEST_BASIC_THREAD 1

class ThreadBase : public NonCopyable
{
public:
	ThreadBase() = default;

	template<class T>
	ThreadBase(T* p);

	~ThreadBase();

	void join();
	void setThreadAffinity(int k_th_bit);

	template<class T>
	void startProc(T* param)
	{
		onRun(param);
	}

	template<class T>
	void startProc(int k_th_bit, T* param)
	{
		_localId = k_th_bit;
		onStartProc(param);
		setThreadAffinity(k_th_bit);
	}

	size_t threadId() const { return _threadId; }
	int    localId() const	{ return _localId; }

protected:
	template<class T>
	void onStartProc(T* param) { _init<T>(param); }

private:
	template<class T>
	static DWORD WINAPI _proc(void* p);

	template<class T>
	void _init(T* param);

protected:

	size_t _threadId = ~size_t(0);

#if SGE_TEST_BASIC_THREAD

#if SGE_OS_WINDOWS
	HANDLE _handle = nullptr;
#else
	int _handle = -1;
#endif

#else
	std::thread _handle;
#endif // SGE_TEST_BASIC_THREAD

	int _localId = -1;
};

#if 0
#pragma mark --- ThreadBase-Impl ---
#endif // 0
#if 1

template<class T> inline
ThreadBase::ThreadBase(T* p)
{
	onRun<T>(p);
}

template<class T> inline
DWORD ThreadBase::_proc(void* p)
{
	auto* obj = reinterpret_cast<T*>(p);
	obj->onProc();
	return 0;
}

template<class T> inline
void ThreadBase::_init(T* param)
{
	SGE_ASSERT(param == this);
	
#if SGE_TEST_BASIC_THREAD
	_handle		= ::CreateThread(nullptr, 0, &_proc<T>, (LPVOID)param, 0, (LPDWORD)&_threadId);
	//_threadId	= ::GetThreadId(_handle);
	SGE_ASSERT(_handle);
#else
	_handle = std::thread(&_proc<T>, (LPVOID)param);
	_threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());

#endif // 0


#if 0
	static size_t n = 0;
	_localId = n;
	atomicLog("thread id: {}", n);
	++n;
#endif // 1

}

#endif // 1

}