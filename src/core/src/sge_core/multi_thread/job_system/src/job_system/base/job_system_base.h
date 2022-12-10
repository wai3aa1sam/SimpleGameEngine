#pragma once

#include <sge_core-pch.h>

#include <sge_core/multi_thread/atomic/Synchronize.h>
#include <sge_core/multi_thread/atomic/AtomicQueue.h>

#include <sge_core/multi_thread/atomic/ThreadBase.h>

#include <random>

#include <sge_core/base/sge_macro.h>
#include <sge_core/log/Log.h>

#include <EASTL/algorithm.h>
#include <EASTL/sort.h>

namespace sge {

#define SGE_JOB_SYSTEM_ENABLE_THREAD_TYPE 0
#define SGE_JOB_SYSTEM_ENABLE_SINGLE_THREAD_DEBUG 0
#define SGE_JOB_SYSTEM_DEBUG 1

#if SGE_JOB_SYSTEM_ENABLE_SINGLE_THREAD_DEBUG

#define ThreadType_ENUM_LIST(E) \
	E(Count, = 16)	\
//----------
SGE_ENUM_CLASS(ThreadType, u8)

#else

#if SGE_JOB_SYSTEM_ENABLE_THREAD_TYPE

#define ThreadType_ENUM_LIST(E) \
	E(Main,)	\
	E(Render,)	\
	E(Game,)	\
	E(Worker,)	\
	E(Count,)	\
//----------
SGE_ENUM_CLASS(ThreadType, u8)

#else

#define ThreadType_ENUM_LIST(E) \
	E(Count, = 0)	\
//----------
SGE_ENUM_CLASS(ThreadType, u8)

#endif // 0

#endif // SGE_JOB_SYSTEM_SINGLE_THREAD_DEBUG

#define JobPrioity_ENUM_LIST(E) \
	E(Cirtical, = 0)	\
	E(High,)			\
	E(Normal,)			\
	E(Low,)				\
	E(Count,)			\
//----------
SGE_ENUM_CLASS(JobPrioity, u8)

template<class T> using PrioityQueue = PrioityQueues<T, enumInt(JobPrioity::Count)>;

static constexpr size_t s_kCacheLine = 64;

extern thread_local i32 _threadLocalId;
inline i32 threadLocalId() { return _threadLocalId; }

template<class T> inline 
void swap(T*& a, T*& b) 
{
	T* tmp = a;
	a = b;
	b = tmp;
}

template<class T>
class TypeDisplayer;
// eg. TypeDisplayer<decltype(data->data)> x; (void)x;

template<class... ARGS> inline
constexpr auto make_tuple(ARGS&&... args)
{
	return std::make_tuple(std::forward<ARGS>(args)...);
}

template<class T> inline
bool isPowOfTwo(T x)  { return !(x == 0) && !(x & (x - 1)); }


class Random : public NonCopyable
{
public:
	template<class T>
	T get(T min, T max)
	{
		static_assert(!(std::is_same_v<T, float> || std::is_same_v<T, double>), "should use std::uniform_real_distribution<float>");

		std::uniform_int_distribution<T> uni(min, max);
		return uni(_rng);
	}

	template<class T>
	T getIfNot(T min, T max, T not)
	{
		T val = get(min, max);
		while (val == not)
			val = get(min, max);
		return val;
	}

	template<class T>
	static T s_get(T min, T max)
	{
		static Random r;
		return r.get(min, max);
	}

private:
	std::default_random_engine _rng { std::random_device{}() };
};

template<class T, size_t TARGET = sizeof(T), bool SAME_SIZE = TARGET == sizeof(T)>
struct Padding
{
};

template<class T, size_t TARGET>
struct Padding<T, TARGET, false>
{
private:
	u8 _padding[TARGET - sizeof(T)];
};

}