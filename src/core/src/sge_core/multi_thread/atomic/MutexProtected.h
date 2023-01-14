#pragma once
#include "Synchronize.h"

namespace sge {
template<class T, class MUTEX, class LOCK>
class Locked_T
{
	using Mutex = MUTEX;
	using Lock	= LOCK;
public:
	Locked_T(T& data, Mutex& mutex)
		:
		_lock(mutex), _data(&data)
	{
	}

	Locked_T(Locked_T&& r)
		: _lock(std::move(r._lock))
	{
		_data = r._data;
		r._data = nullptr;
	}

	T* operator->() { return  _data; }
	T& operator*()	{ return *_data; }

private:
	Lock	_lock;			// should lock first
	T*		_data = nullptr;
};


template<class T, class MUTEX, class ENABLE = void>
class MutexProtected_Data_T
{
protected:
	using Mutex = MUTEX;
	using ULock = ULock_T<Mutex>;
	using SLock = SLock_T<Mutex>;

protected:
	Mutex	_mutex;
	T		_data;
};

template<class T, class MUTEX, class DATA, class ENABLE = void /*std::enable_if<std::is_same<MUTEX, Mutex>::value>::type*/ >
class MutexProtected_T : public DATA
{
public:
	using Mutex = typename DATA::Mutex;
	using ULock = typename DATA::ULock;
	using SLock = typename DATA::SLock;
	
	SGE_NODISCARD Locked_T<T, Mutex, ULock> scopedULock() { return { _data, _mutex }; }
	
protected:
	using DATA::_mutex;
	using DATA::_data;
};

// type trait wrapper failed
template<class T, class MUTEX, class DATA>
class MutexProtected_T<T, MUTEX, DATA, std::enable_if_t<std::is_same<MUTEX, SMutex>::value> > : public DATA
{
public:
	using Mutex = typename DATA::Mutex;
	using ULock = typename DATA::ULock;
	using SLock = typename DATA::SLock;

	SGE_NODISCARD Locked_T<T, Mutex, ULock> scopedULock() { return { _data, _mutex }; }
	SGE_NODISCARD Locked_T<T, Mutex, SLock> scopedSLock() { return { _data, _mutex }; }

protected:
	using DATA::_mutex;
	using DATA::_data;
};

template<class T> using MutexProtected	= MutexProtected_T<T, Mutex,	MutexProtected_Data_T<T, Mutex>  >;
template<class T> using SMutexProtected = MutexProtected_T<T, SMutex,	MutexProtected_Data_T<T, SMutex> >;

template<class T, class MUTEX, class CONDVAR, class LOCK>
class LockedCV_T
{
	using Mutex		= MUTEX;
	using CondVar	= CONDVAR;
	using Lock		= LOCK;
	//using Locked	= Locked_T<T, Mutex, Lock>;
	//using ULocked	= Locked_T<T, Mutex, ULock>;
	//using SLocked	= Locked_T<T, Mutex, SLock>;

public:
	LockedCV_T(T& data, Mutex& mutex, CondVar& condvar)
		:
		_lock(mutex), _condvar(&condvar), _data(&data)
	{
	}

	LockedCV_T(LockedCV_T && r) 
		: _lock(std::move(r._lock))
	{
		_condvar	= r._condvar;
		_data		= r._data;
		r._condvar	= nullptr;
		r._data		= nullptr;
	}

	T* operator->() { return  _data; }
	T& operator*()  { return *_data; }

	void wait() 
	{ 
		if (_condvar)
		{
			_condvar->wait(_lock); 
		}
	};

private:
	//Locked_T _locked; // should lock first
	Lock		_lock;
	CondVar*	_condvar = nullptr;
	T*			_data = nullptr;
};

template<class T, class MUTEX, class CONDVAR, class ENABLE = void>
class CondVarProtected_Data_T
{
protected:
	using Mutex		= MUTEX;
	using CondVar	= CONDVAR;
	using ULock		= ULock_T<Mutex>;
	using SLock		= SLock_T<Mutex>;

public:
	void notifyAll()			{ _condvar.notify_all(); }

protected:
	Mutex	_mutex;
	CondVar	_condvar;
	T		_data;
};

template<class T, class MUTEX, class CONDVAR, class DATA, class ENABLE = void /*std::enable_if<std::is_same<MUTEX, Mutex>::value>::type*/ >
class CondVarProtected_T : public DATA
{
public:
	using Mutex		= typename DATA::Mutex;
	using CondVar	= typename DATA::CondVar;
	using ULock		= typename DATA::ULock;
	using SLock		= typename DATA::SLock;

	SGE_NODISCARD LockedCV_T<T, Mutex, CondVar, ULock> scopedULock() { return { _data, _mutex, _condvar }; }

protected:
	using DATA::_mutex;
	using DATA::_condvar;
	using DATA::_data;
};


// type trait wrapper failed
template<class T, class MUTEX, class CONDVAR, class DATA>
class CondVarProtected_T<T, MUTEX, CONDVAR, DATA, std::enable_if_t<std::is_same<MUTEX, SMutex>::value> > : public DATA
{
public:
	using Mutex		= typename DATA::Mutex;
	using CondVar	= typename DATA::CondVar;
	using ULock		= typename DATA::ULock;
	using SLock		= typename DATA::SLock;

	SGE_NODISCARD LockedCV_T<T, Mutex, CondVar, ULock> scopedULock() { return { _data, _mutex, _condvar}; }
	SGE_NODISCARD LockedCV_T<T, Mutex, CondVar, SLock> scopedSLock() { return { _data, _mutex, _condvar}; }


protected:
	using DATA::_mutex;
	using DATA::_condvar;
	using DATA::_data;
};

template<class T> using CondVarProtected		= CondVarProtected_T<T, Mutex,	CondVar,	CondVarProtected_Data_T<T, Mutex,  CondVar>  >;
template<class T> using SMtxCondVarProtected	= CondVarProtected_T<T, SMutex,	CondVarA,	CondVarProtected_Data_T<T, SMutex, CondVarA> >;


}