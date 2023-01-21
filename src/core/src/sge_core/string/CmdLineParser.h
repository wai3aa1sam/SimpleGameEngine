#pragma once
#include "Lexer.h"
#include <sge_core/file/FileStream.h>

namespace sge {

struct CmdLineArg
{
	CmdLineArg(int argc, char** argv)
		:
		argc(argc), argv(argv)
	{
	}
	void stream(TempString& str) const
	{
		str.clear();
		for (int i = 0; i < argc; i++)
		{
			str.append(argv[i]);
			str.append(" ");
		}
	}

	int count() const { return argc; }

	const char* operator[](size_t idx)			{ return argv[idx]; }
	const char* operator[](size_t idx) const	{ return argv[idx]; }

	StrView current() const 
	{
		if (!isValid())
			return StrView();
		return StrView(argv[_cur]);
	}

	StrView next() const
	{ 
		if (!isValid())
			return StrView();

		auto o = StrView(argv[_cur]);
		_cur++;
		return o;
	}

	int		argc = 0;
	char**	argv = nullptr;

protected:
	bool isValid() const { return _cur < argc; }

private:
	mutable int _cur = 0;
};

class CmdLineParser : public Lexer
{
public:


protected:
};

}