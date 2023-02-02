#include <sge_core-pch.h>
#include "Process.h"

namespace sge {

#if 0
#pragma mark --- Process-Impl
#endif // 0
#if 1

#if SGE_OS_WINDOWS

Process::Process(StrView filename)
{
	_init(filename, "");
}

Process::Process(StrView filename, StrView args)
{
	_init(filename, args);
}


Process::~Process()
{
	clear();
}

void Process::create(StrView filename, StrView args)
{
	_init(filename, args);
}

void Process::clear()
{
	WaitForSingleObject(_procInfo.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(_procInfo.hProcess);
	CloseHandle(_procInfo.hThread);

	_startupInfo	= { 0 };
	_procInfo		= { 0 };
}

void Process::_init(StrView filename, StrView args)
{
	clear();

	TempStringW filenameW;
	UtfUtil::convert(filenameW, filename);

	TempStringW argsW;
	UtfUtil::convert(argsW, args);

	_startupInfo.cb = sizeof(_startupInfo);

	auto ret = ::CreateProcess(
		filenameW.c_str(),		// No module name (use command line)
		(LPWSTR)argsW.c_str(),	// Command line
		NULL,					// Process handle not inheritable
		NULL,					// Thread handle not inheritable
		FALSE,					// Set handle inheritance to FALSE
		0,						// No creation flags
		NULL,					// Use parent's environment block
		NULL,					// Use parent's starting directory 
		&_startupInfo,			// Pointer to STARTUPINFO structure
		&_procInfo				// Pointer to PROCESS_INFORMATION structure
	);

	if (!ret)
	{
		throw SGE_ERROR("cannot execute process {}", filename);
	}
}


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


#if SGE_OS_WINDOWS


Shell::Shell(StrView filename)
{
	_init(filename, "");
}

Shell::Shell(StrView filename, StrView args)
{
	_init(filename, args);
}

Shell::~Shell()
{
	clear();
}

void Shell::create(StrView filename, StrView args)
{
	_init(filename, args);
}

void Shell::_init(StrView filename, StrView args)
{
	clear();

	TempStringW filenameW;
	UtfUtil::convert(filenameW, filename);

	TempStringW argsW;
	UtfUtil::convert(argsW, args);

	_shExecInfo = {0};
	_shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	_shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	_shExecInfo.hwnd = NULL;
	_shExecInfo.lpVerb = L"open";
	_shExecInfo.lpFile = filenameW.c_str();
	_shExecInfo.lpParameters = argsW.c_str();
	_shExecInfo.lpDirectory = NULL;
	_shExecInfo.nShow = SW_NORMAL;
	_shExecInfo.hInstApp = NULL; 
	ShellExecuteEx(&_shExecInfo);
}

void Shell::clear()
{
	WaitForSingleObject(_shExecInfo.hProcess, INFINITE);
	CloseHandle(_shExecInfo.hProcess);

	_shExecInfo = { 0 };
}

#elif SGE_OS_LINUX

#elif SGE_OS_MACOSX

#else
#error "unknown-platform"
#endif // SGE_OS_WINDOWS



#endif

}