#pragma once
#include "Synchronize.h"

namespace sge {

template<class T>
class Locked
{
public:
	Locked(T& data, Mutex& mutex)
		:
		_lock(mutex), _data(&data)
	{
	}

	Locked(Locked && r)
		: _lock(std::move(r._lock))
	{
		_data	= r._data;
		r._data = nullptr;
	}

	T* operator->() { return _data; }

private:
	Lock _lock;			// should lock first
	T* _data = nullptr;
};

template<class T>
class MutexProtected
{
public:
	template <class... ARGS>
	MutexProtected(ARGS&&... args)
		:
		_data(std::forward<ARGS>(args)...)
	{
	}
	Locked<T> scopedLock() { return {_data, _mutex}; }

private:
	Mutex	_mutex;
	T		_data;
};

template<class T>
class LockedCV
{
public:
	LockedCV(T& data, Mutex& mutex, CondVar& condvar)
		:
		_lock(mutex), _condvar(&condvar), _data(&data)
	{
	}

	LockedCV(LockedCV && r) 
		: _lock(std::move(r._lock))
	{
		_condvar	= r._condvar;
		_data		= r._data;
		r._condvar	= nullptr;
		r._data		= nullptr;
	}

	T* operator->() { return _data; }
	T& operator*()  { return *_data; }


	void wait() 
	{ 
		if (_condvar)
		{
			_condvar->wait(_lock); 
		}
	};


private:
	Lock _lock;			// should lock first
	CondVar* _condvar = nullptr;
	T* _data = nullptr;
};

template<class T>
class ConVarProtected
{
public:
	template<class... ARGS>
	ConVarProtected(ARGS&&... args)
		:
		_data(std::forward<ARGS>(args)...)
	{
	}

	LockedCV<T> scopedLock()	{ return {_data, _mutex, _condvar}; }
	void notifyAll()			{ _condvar.notify_all(); }

private:
	Mutex	_mutex;
	CondVar	_condvar;
	T		_data;
};

}