#pragma once

#include "../base/job_system_base.h"

#include <memory>
#include <functional>

namespace sge {

#if 0
#pragma mark --- LocalBuffer_T-Impl
#endif // 0
#if 1

template<size_t LOCAL_SIZE, size_t ALIGN = s_kAlignment>
class LocalBuffer_Base
{
public:
	using LocalBufferType = typename std::aligned_storage<LOCAL_SIZE, ALIGN>::type;
	using SizeType = size_t;

	static constexpr SizeType s_kLocalSize			= LOCAL_SIZE;
	static constexpr SizeType s_kAlign				= ALIGN;

	using This = LocalBuffer_Base<s_kLocalSize, s_kAlign>;

public:
	SGE_NODISCARD void* alloc(SizeType n)
	{
		if (n > s_kLocalSize)
			return nullptr;
		return &_buf;
	}

	void free(void* p, SizeType n)
	{

	}

			void* data()		{ return &_buf; }
	const	void* data() const	{ return &_buf; }

	bool isUsingLocalBuffer(void* p) const { return p == data(); }

	//void clear()			{ ::memset(&_buf, 0, n);  /* TODO: our impl for memset(like memcpy)*/ }
	//bool isEmpty() const	{ return &_buf == nullptr; }
private:
	LocalBufferType _buf;
};

template<> 
class LocalBuffer_Base<0, 0> 
{
public:
	using LocalBufferType = void;
	using SizeType = size_t;

	static constexpr SizeType s_kLocalSize			= 0;
	static constexpr SizeType s_kAlign				= 0;
	
	using This = LocalBuffer_Base<s_kLocalSize, s_kAlign>;

public:
	SGE_NODISCARD void* alloc(SizeType n)
	{
		return nullptr;
	}

	void free(void* p, SizeType n)
	{

	}

			void* data()		{ return nullptr; }
	const	void* data() const	{ return nullptr; }

	bool isUsingLocalBuffer(void* p) const { return false; }

	//void clear()			{ ::memset(&_buf, 0, n);  /* TODO: our impl for memset(like memcpy)*/ }
	//bool isEmpty() const	{ return &_buf == nullptr; }
};

template<size_t LOCAL_SIZE, size_t ALIGN = s_kAlignment, bool ENABLE_FALLBACK_ALLOC = true, class FALLBACK_ALLOCATOR = DefaultAllocator_T<ALIGN>>
class LocalBuffer_T
{
public:
	using LocalBufferType = LocalBuffer_Base<LOCAL_SIZE, ALIGN>;
	using SizeType = size_t;
	using FallbackAllocator = FALLBACK_ALLOCATOR;

	template <class T1, class T2> using CompressedPair = std::pair<T1, T2>;

	static constexpr SizeType s_kLocalSize			= LOCAL_SIZE;
	static constexpr SizeType s_kAlign				= ALIGN;
	static constexpr bool	  s_enableFallbackAlloc = ENABLE_FALLBACK_ALLOC;
	//static constexpr bool	  s_isAllocator			= IS_ALLOCATOR;

	using This = LocalBuffer_T<s_kLocalSize, s_kAlign, s_enableFallbackAlloc, FallbackAllocator>;

public:
	LocalBuffer_T() = default;
	~LocalBuffer_T() = default;

	SGE_NODISCARD void* alloc(SizeType n)
	{
		auto& localBuf			= _pair.first;
		auto& fallbackAllocator = _pair.second;

		if (n > s_kLocalSize)
			return fallbackAllocator.alloc(n);
		return localBuf.alloc(n);
	}

	void free(void* p, SizeType n)
	{
		SGE_ASSERT(!isUsingLocalBuffer(p), "cannot free local buffer pointer");

		auto& fallbackAllocator = _pair.second;
		fallbackAllocator.free(p, n);
	}

	bool isUsingLocalBuffer(void* p) const { auto& localBuf = _pair.first; return localBuf.isUsingLocalBuffer(p); }

private:
	CompressedPair<LocalBufferType, FALLBACK_ALLOCATOR> _pair;
};

template<size_t LOCAL_SIZE, size_t ALIGN>
class LocalBuffer_T<LOCAL_SIZE, ALIGN, false> : public LocalBuffer_Base<LOCAL_SIZE, ALIGN>
{
public:
	using LocalBufferType = LocalBuffer_Base<LOCAL_SIZE, ALIGN>;
	using SizeType = size_t;

	static constexpr SizeType s_kLocalSize			= LOCAL_SIZE;
	static constexpr SizeType s_kAlign				= ALIGN;
	static constexpr bool	  s_enableFallbackAlloc = false;
	//static constexpr bool	  s_isAllocator			= IS_ALLOCATOR;

	using Base = LocalBufferType;
	using This = LocalBuffer_T<s_kLocalSize, s_kAlign, s_enableFallbackAlloc>;

	LocalBuffer_T() = default;
	~LocalBuffer_T() = default;
};

template<> class LocalBuffer_T<0> : public LocalBuffer_Base<0, 0> {};

#endif // 0


#if 0
#pragma mark --- Function_T-Impl
#endif // 0
#if 1

template <class T1, class T2> using EnableIfNotFunction_T = std::enable_if_t< !std::is_same_v< std::decay_t<T1>, std::decay_t<T2> > >;

template<class T, size_t LOCAL_SIZE = 32, size_t ALIGN = s_kAlignment, bool ENABLE_FALLBACK_ALLOC = true, class FALLBACK_ALLOCATOR = DefaultAllocator> // TODO: change  std::allocator<u8> to our own fallback allocator
class Function_T;

template<class RET, class... PARAMS, size_t LOCAL_SIZE, size_t ALIGN, bool ENABLE_FALLBACK_ALLOC, class FALLBACK_ALLOCATOR>
class Function_T<RET(PARAMS...), LOCAL_SIZE, ALIGN, ENABLE_FALLBACK_ALLOC, FALLBACK_ALLOCATOR> /*: public NonCopyable*/
{
public:
	using FallbackAllocator = FALLBACK_ALLOCATOR;
	using SizeType			= size_t;

	static constexpr SizeType s_kLocalSize			= LOCAL_SIZE;
	static constexpr SizeType s_kAlign				= ALIGN;
	static constexpr bool	  s_enableFallbackAlloc = ENABLE_FALLBACK_ALLOC;

	using LocalBuffer	= LocalBuffer_T<s_kLocalSize, s_kAlign, s_enableFallbackAlloc, FallbackAllocator>;
	using This			= Function_T<RET(PARAMS...), s_kLocalSize, s_kAlign, s_enableFallbackAlloc, FallbackAllocator>;

	using FuncType									= RET(PARAMS...);
	template<class FUNC> using EnableIfNotFunction	= EnableIfNotFunction_T<FUNC, Function_T>;
	//template <class T> using UPtr					= eastl::unique_ptr<T, UPtr_Deleter<LocalBuffer> >
	class IFunctor;

public:
	Function_T() noexcept
		: _ftr(nullptr)
	{}

	Function_T(nullptr_t) noexcept
		: _ftr(nullptr)
	{}

	Function_T(const Function_T& rhs)
	{
		_clone(rhs);
	}

	Function_T(Function_T&& rhs) noexcept
	{
		_move(std::move(rhs));
	}

	template<class FUNC, class = EnableIfNotFunction<FUNC>>
	Function_T(FUNC&& func)
	{
		_ctor(std::forward<FUNC>(func));
	}

	void operator=(const Function_T& rhs) 
	{
		_free();
		_clone(rhs);
}

	void operator=(Function_T&& rhs) noexcept
	{
		_free();
		_move(std::move(rhs));
	}

	template<class FUNC, class = EnableIfNotFunction<FUNC>>
	void operator=(FUNC&& func) noexcept
	{
		_free();
		_ctor(std::forward<FUNC>(func));
	}

	~Function_T()
	{
		_free();
	}

	RET operator()(PARAMS&&... params)
	{
		return _ftr->operator()(std::forward<PARAMS>(params)...);
	}

	explicit operator bool() const 
	{
		return _ftr != nullptr;
	}

	void operator=(nullptr_t) 
	{
		_reset();
	}

	bool operator==(nullptr_t) const
	{
		return _ftr == nullptr;
	}

	/*friend bool operator==(const Function_T& f1, const Function_T& f2) 
	{
		return f1._ftr == f2._ftr;
	}

	friend bool operator!=(const Function_T& f1, const Function_T& f2) 
	{
		return !(f1 == f2);
	}*/

private:
	template<class FUNC = void>
	static constexpr void staticCheck()
	{
		//static_assert(IsSame<FUNC, void>|| FUNC is callable, "FUNC in Function is not a callable");
		//static_assert(std::is_convertible_v<FUNC, decltype(std::function<FuncType>)>,	"Function_T: Wrong Signature!");
		//static_assert(IsFunction<FUNC>,													"Function_T: Type is not a function!");
		static_assert(sizeof(FUNC) <= s_kLocalSize || s_enableFallbackAlloc,			"Function_T: localBuf overflow");
	}

	template<class FUNC>
	void _ctor(FUNC&& func)
	{
		using Func		= std::decay_t<FUNC>;
		using Functor	= Functor<Func>;
		staticCheck<Functor>();
		
		auto* buf	= _localBuf.alloc(sizeof(Functor));
		auto* pfunc = new(buf) Functor(std::forward<Func>(func));
		_reset(pfunc);
	}

	void _clone(const Function_T& rhs)
	{
		if (this == &rhs) return;
		IFunctor* pfunc = nullptr;
		if (rhs)
		{
			auto* buf	= _localBuf.alloc(rhs._ftr->size());
			pfunc		= rhs._ftr->clone(buf);
		}
		_reset(pfunc);
	}

	void _move(Function_T&& rhs)
	{
		if (this == &rhs) return;
		IFunctor* pfunc = nullptr;
		if (rhs)
		{
			auto* buf	= _localBuf.alloc(rhs._ftr->size());
			pfunc		= rhs._ftr->clone(buf);
		}
		_reset(pfunc);

		rhs._free();
	}
	
	void _free()
	{
		if (_ftr && !_localBuf.isUsingLocalBuffer(_ftr))
		{
			_localBuf.free(_ftr, 0);
		}
		_reset();
	}

	void _reset(IFunctor* p = nullptr)
	{
		_ftr = p;
	}

private:

	//template<class RET, class... PARAMS, class FALLBACK_ALLOCATOR>
	class IFunctor
	{
	public:
		virtual ~IFunctor()							= default;
		virtual RET operator()(PARAMS&&...) const	= 0;
		virtual IFunctor* clone(void* buf) const	= 0;

		virtual SizeType size() const = 0;
	};

	template<class FUNC>
	class Functor final : public IFunctor
	{
	public:
		template<typename FUNCTOR>
		Functor(FUNCTOR&& func)
			: _func(std::forward<FUNCTOR>(func))
		{}

		/*Functor(FUNC func)
			: _func(func)
		{}*/

		~Functor()
		{
			SGE_LOG("~Functor");
		}

		//virtual RET operator()(PARAMS&&... params) const override { return _func(std::forward<PARAMS>(params)...); }
		virtual RET operator()(PARAMS&&... params) const override	{ return std::invoke(_func, std::forward<PARAMS>(params)...); }

		virtual IFunctor* clone(void* buf) const override 
		{
			staticCheck<Functor>();
			//SGE_DUMP_VAR(sizeof(FUNC));
			//SGE_DUMP_VAR(sizeof(Functor));
			//SGE_DUMP_VAR(sizeof(_func));

			return new(buf) Functor(_func); 
		};

		virtual SizeType size() const 
		{ 
			return sizeof(Functor);
			//return sizeof(Functor) > sizeof(FUNC) ? sizeof(Functor) : sizeof(FUNC); 
		}

	private:
		FUNC _func;
	};

private:
	LocalBuffer	_localBuf;
	IFunctor*	_ftr = nullptr;
};

#endif // 0

struct Test1_Base
{

};

template<class T>
struct Test1 : Test1_Base
{
	Test1(T t) : func(t) 
	{
		SGE_LOG("===");
		SGE_DUMP_VAR(sizeof(Test1));
		SGE_DUMP_VAR(sizeof(T));
		SGE_LOG("===");
	}

	T func;
};

struct Test1_VBase
{
	virtual ~Test1_VBase()
	{

	}
};

template<class T>
struct Test1V : public Test1_VBase
{
	Test1V(T t) : func(t) 
	{
		SGE_LOG("===");
		SGE_DUMP_VAR(sizeof(Test1V));
		SGE_DUMP_VAR(sizeof(T));
		SGE_LOG("===");
	}

	T func;
};


}