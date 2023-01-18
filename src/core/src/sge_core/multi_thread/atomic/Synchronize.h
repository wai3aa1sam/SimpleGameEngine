#pragma once
#include <sge_core-pch.h>

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#include <signal.h>

#include <functional>

#include "EASTL/deque.h"
#include "EASTL/queue.h"

#include <future>

#include <sge_core/log/Log.h>

namespace sge {

using Mutex		= std::mutex;
using SMutex	= std::shared_mutex;

template<class T> using ULock_T		= std::unique_lock<T>;
template<class T> using SLock_T		= std::shared_lock<T>;

using ULock		= ULock_T<Mutex>;
using SLock		= SLock_T<SMutex>;

template<class... T> using ALock = std::unique_lock<T...>;

using CondVar	= std::condition_variable;
using CondVarA	= std::condition_variable_any;

using Thread = std::thread;

template<class T>	using Queue = eastl::queue<T>;
template<class T>	using Deque = eastl::deque<T>;

//template<class T>	using Function = std::function<T>;

template<class T>	using Atomic = std::atomic<T>;

static constexpr size_t s_kCahchLineSize = std::hardware_destructive_interference_size;

template<class T> using UnderLyingType = typename std::underlying_type<T>::type;


template<class T> using Future = std::future<T>;
template<class T> using Promise = std::promise<T>;

extern bool s_isQuit;

inline size_t logicalThreadCount()
{
	return std::thread::hardware_concurrency();
}

inline void sleep(int sec_)
{
#if SGE_OS_WINDOWS
	::Sleep(sec_ * 1000);
#else
	::sleep(sec_);
#endif // TLC_OS_WINDOWS
}

inline void sleep_ms(int ms_)
{
#if SGE_OS_WINDOWS
	::Sleep(ms_);
#else
	::sleep(ms_);
#endif // TLC_OS_WINDOWS
}

inline void my_singal_handler(int sig) {
	printf("my_singal_handler %d\n", sig);
	switch (sig) {
	case SIGINT:
	case SIGTERM: {
		//s_isQuit = true;
	}break;
	}
}


template<class... ARGS> inline
void atomicLog(ARGS&&... args)
{
	static Mutex mutex;
	ULock lock(mutex);
	SGE_LOG(SGE_FORWARD(args)...);

	//TempString buffer;
	//fmt::format_to(std::back_inserter(buffer), SGE_FORWARD(args)...);
	//std::cout << buffer.c_str() << "\n";
}

class MyTimer {
public:
	MyTimer() { 
		reset(); 
	}
	~MyTimer()
	{
		print();
	}

	void reset() {
		m_start = getTick();
	}

	double get() {
		auto c = getTick();
		return (c - m_start) / 1000.0f;
	}

	void print() {
		printf("time: %f\n", get());
	}

private:
	uint64_t getTick() { 
		return GetTickCount64();
	}

	uint64_t m_start;
};

}