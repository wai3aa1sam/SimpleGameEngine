#pragma once
#include <sge_core/string/CmdLineParser.h>
#include "CompileRequest.h"

namespace sge {

class ShaderCmdLineParser : public CmdLineParser
{
	using Request = CompileRequest;
	using Info = CompileInfo;
public:
	void readArgv(Info& info, const CmdLineArg& arg);


private:
	void _readCmdLine();

	void _readCWD();
	void _readShaderLang();
	void _readFile();
	void _readOutput();

	void _readProfile();
	void _readEntry();
	void _readInclude();
	void _readMarco();

	void _skipProgramName();
	void _skipAssign();

	bool isFirstChar(StrView sv) { return token().str[0] == sv[0]; }
	void extract(String& out, StrView target, StrView delimiter);

	const Token& token() const { return CmdLineParser::token(); }

	void _error(StrView sv) { throw SGE_ERROR("Error: {}", sv); }

	char getCmdCh() const { return _cmdCh; }

	bool isCmd(StrView sv) { return sv.compare(_buf) == 0; }

	void cmdExtract(String& out, StrView delim)
	{
		out.clear();
		for (;;)
		{
			auto* p = delim.begin();
			auto* e = delim.end();
			for (; p < e; p++) {
				if (_cmdCh == *p)
				{
					nextCmdChar();
					return;
				}
				out += _cmdCh;
			}

			if (!nextCmdChar())
				break;
		}

	}
	void cmdExtract(StrView delim)
	{
		cmdExtract(_buf, delim);
	}

	void cmdRead(String& out)
	{
		out.clear();
		auto remain = getRemainCmd();
		out += remain;
	}

	bool nextCmdChar()
	{
		_cmdCh = 0;
		if (!_cmdCur) return false;
		if (_cmdCur >= _cmd.end()) return false;

		_cmdCh = *_cmdCur;
		_cmdCur++;

		return true;
	}

	bool cmdSkipTo(StrView delim)
	{
		for (;;)
		{
			auto* p = delim.begin();
			auto* e = delim.end();
			for (; p < e; p++) {
				if (_cmdCh == *p)
					return true;
			}

			if (!nextCmdChar())
				break;
		}
		return false;
	}

	bool cmdSkipToNext(StrView delim)
	{
		bool ret = cmdSkipTo(delim);
		nextCmdChar();
		return ret;
	}

	StrView nextCmd() 
	{ 
		_cmd = _cmdArg->next();
		_cmdCur = _cmd.begin();
		nextCmdChar();
		return _cmd; 
	}

	StrView cmd() { return _cmd; }

	StrView getRemainCmd() const {
		if (!_cmdCur) return StrView();
		auto* s = _cmdCur - 1;
		return StrView(s, _cmd.end() - s);
	}

	#if 0
	void nextToken()				{ _lexer.nextToken(); }
	Lexer::Token& token()			{ _lexer.getToken(); }
	void expectOperator(StrView sv) { _lexer.expectOperator(sv); }
	#endif // 0

	Info&		info()	{ return *_info; }
	Request&	req()	{ return _info->comileRequest; }

private:
	Info* _info = nullptr;
	const CmdLineArg* _cmdArg = nullptr;

	StrView _cmd;
	const char* _cmdCur = nullptr;
	char _cmdCh = 0;

	String _buf;
};

}