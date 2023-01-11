#pragma once

#include <sge_core-pch.h>

#include <sge_core/multi_thread/atomic/Synchronize.h>
#include <sge_core/multi_thread/atomic/AtomicQueue.h>

#include <sge_core/multi_thread/atomic/ThreadBase.h>

#include <random>

#include <sge_core/base/sge_macro.h>
#include <sge_core/log/Log.h>

#include <sge_core/profiler/sge_profiler.h>

#include <EASTL/algorithm.h>
#include <EASTL/sort.h>

namespace sge {

#if _DEBUG

#define SGE_JOB_SYSTEM_BUILD_CONFIG_RELEASE 0

#else

#define SGE_JOB_SYSTEM_BUILD_CONFIG_RELEASE 1

#endif // _DEBUG


#define SGE_JOB_SYSTEM_ENABLE_THREAD_TYPE 0
#define SGE_JOB_SYSTEM_ENABLE_SINGLE_THREAD_DEBUG 0
#define SGE_JOB_SYSTEM_DEBUG 0

#define SGE_JOB_SYSTEM_IS_CONDITION_DEBUG 1

#define SGE_ENABLE_TRACY_PROILER 1

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
	E(Main,  = 0)	\
	E(Count, = 1)	\
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

#define SGE_JOB_SYSTEM_JOB_TYPE_FRIEND_CLASS_DECLARE() \
	template<class T> friend class Job_Base;									\
	template<class T> friend class JobFor_Base;									\
	template<class T> friend class JobParFor_Base;								\
	template<class T, class ENABLE> friend struct JobDispatcher					\
//---

template<class T> using PrioityQueue = PrioityQueues<T, enumInt(JobPrioity::Count)>;

static constexpr size_t s_kCacheLine = 64;
static constexpr int s_kIdleSleepTimeMS = 1;
static constexpr int s_kBusySleepTimeMS = 0;

extern thread_local i32 _threadLocalId;
inline i32 threadLocalId() { return _threadLocalId; }

class Job;
using JobHandle = Job*;

#if 0
#pragma mark --- type_trait-Impl
#endif // 0
#if 1

template<class T1, class T2> using					IsSameT = std::is_same<T1, T2>;
template<class T1, class T2> inline constexpr bool	IsSame	= IsSameT<T1, T2>::value;

template<bool COND> using EnableIfT = typename std::enable_if<COND>;
template<bool COND> using EnableIf	= typename EnableIfT<COND>::type;

template<class BASE, class DERIVED> using				  IsBaseOfT = typename std::is_base_of<BASE, DERIVED>;
template<class BASE, class DERIVED> inline constexpr bool IsBaseOf  = IsBaseOfT<BASE, DERIVED>::value;

template<bool COND, class IF_TRUE_T, class IF_FASLE_T> using ConditionalT	= typename std::conditional<COND, IF_TRUE_T, IF_FASLE_T>;
template<bool COND, class IF_TRUE_T, class IF_FASLE_T> using Conditional	= typename ConditionalT<COND, IF_TRUE_T, IF_FASLE_T>::type;

template<class T, T VALUE> using IntConstant = typename std::integral_constant<T, VALUE>;

template<class T> using NumericLimit = typename std::numeric_limits<T>;


#endif // 1

#if 0
#pragma mark --- StackAllocator
#endif // 0
#if 1

template<size_t N> using T_SizeType =	Conditional<IntConstant<bool, N <= NumericLimit< u8>::max() >::value, u8,
										Conditional<IntConstant<bool, N <= NumericLimit<u16>::max() >::value, u16,
										Conditional<IntConstant<bool, N <= NumericLimit<u32>::max() >::value, u32,
										Conditional<IntConstant<bool, N <= NumericLimit<u64>::max() >::value, u64, u64> >>>;

template<size_t N>
class LocalBuffer
{
	using Type = typename std::aligned_storage<N>::type;

	Type _buf;
//	u8 _buf[N];
};

template<> class LocalBuffer<0> {};

template<size_t N>
class StackAllocator;

template<size_t N>
class StackAllocator
{
public:
	//using SizeType = StackAllocator_SizeType<N>;
	static constexpr size_t s_kSize		= N;
	static constexpr size_t s_kAlign	= 4;

	StackAllocator() : _offset(_storage) {}

	void* alloc(size_t n)
	{
		intptr_t new_n = Math::alignTo(n, s_kAlign);
		if (new_n > (_storage + s_kSize) - _offset)
			return nullptr;

		return _offset + n;
	}

	void free(void* p, size_t n)
	{
		if (p + Math::alignTo(n, s_kAlign) == _offset)
			_offset = p;
	}

	void freeAll() { _offset = _storage; }

	bool isOwn(void* p) const { return p >= _storage && p < _storage + s_kSize; }

private:
	u8	_storage[N];
	u8*	_offset = nullptr;
};

template<>
class StackAllocator<0> {};

class DefaultAllocator
{
public:

	void* alloc(size_t n)
	{
		return new u8[n];
	}

	void free(void* p, size_t n)
	{
		delete[] p;
	}

	//void freeAll() { _offset = _storage; }
	//bool isOwn(void* p) const { return p >= _storage && p < _storage + N; }

};

#endif

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