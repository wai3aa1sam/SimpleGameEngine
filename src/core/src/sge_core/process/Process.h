#pragma once
#include <sge_core/string/UtfUtil.h>

namespace sge {

#if 0
#pragma mark --- Process-Impl
#endif // 0
#if 1

#endif

#if 0
#pragma mark --- Shell-Impl
#endif // 0
#if 1

#endif

#if SGE_OS_WINDOWS

#elif SGE_OS_LINUX

#elif SGE_OS_MACOSX

#else
#error "unknown-platform"
#endif // SGE_OS_WINDOWS

class Process : public NonCopyable
{
public:
	Process() = default;
	Process(StrView filename);
	Process(StrView filename, StrView args);
	//template<size_t N_CLI_ARGS>
	//Process(StrView filename, StrView(&args)[N_CLI_ARGS]);

	~Process();

	void create(StrView filename, StrView args);

private:
	void _init(StrView filename, StrView args);

	void clear();

private:
	#if SGE_OS_WINDOWS
	STARTUPINFO			_startupInfo	= { 0 };
	PROCESS_INFORMATION _procInfo		= { 0 };
	#elif SGE_OS_LINUX

	#elif SGE_OS_MACOSX

	#else
	#error "unknown-platform"
	#endif // SGE_OS_WINDOWS
};

class Shell : public NonCopyable
{
public:
	Shell() = default;
	Shell(StrView filename);
	Shell(StrView filename, StrView args);

	~Shell();

	void create(StrView filename, StrView args);

private:
	void _init(StrView filename, StrView args);

	void clear();

private:
	#if SGE_OS_WINDOWS

	SHELLEXECUTEINFO _shExecInfo = { 0 };

	#elif SGE_OS_LINUX

	#elif SGE_OS_MACOSX

	#else
	#error "unknown-platform"
	#endif // SGE_OS_WINDOWS
};



#if 0
#pragma mark --- Process-Impl
#endif // 0
#if 1

#if SGE_OS_WINDOWS

//template<size_t N_ARGS> inline
//Process::Process(StrView filename, StrView(&args)[N_ARGS])
//{
//	TempString cliArgs;
//
//	for (size_t i = 0; i < N_ARGS; i++)
//	{
//		cliArgs += args[i];
//		cliArgs += ' ';
//	}
//	_init(filename, cliArgs);
//}


#elif SGE_OS_LINUX

#elif SGE_OS_MACOSX

#else
#error "unknown-platform"
#endif // SGE_OS_WINDOWS


#endif

#if 0
#pragma mark --- Shell-Impl
#endif // 0
#if 1

#endif


}