#include "sge_shader_compiler-pch.h"
#include "ShaderCmdLineParser.h"

#define SGE_ShaderCmdLineParser_STREAM_IMPL 0

#define SGE_ShaderCmdLineParser_Debug 0

#if SGE_ShaderCmdLineParser_Debug
#define SGE_ShaderCmdLineParser_PRINT 0
#endif // SGE_ShaderCmdLineParser_Debug


namespace sge {

void ShaderCmdLineParser::readArgv(Info& info, const CmdLineArg& arg)
{
	TempString cmdStream;
	arg.stream(cmdStream);

	_info = &info;
	_cmdArg = &arg;

	reset(cmdStream, "");

	req().language = "hlsl";
	_readCmdLine();

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== End read cmd line =========");
	#endif // SGE_ShaderCmdLineParser_PRINT

}

void ShaderCmdLineParser::_readCmdLine()
{
	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readCmdLine =========");
	#endif // SGE_ShaderCmdLineParser_PRINT

	_skipProgramName();

	#if !SGE_ShaderCmdLineParser_STREAM_IMPL
	char ch = 0;
	for (;; nextCmd())
	{
		ch = getCmdCh();
		if (!ch)
			break;
		if (!cmd().size())
			break;

		if (ch != '-')
			_error("in correct syntax, - is missing");

		nextCmdChar();
		ch = getCmdCh();

		// "-[cmd]=xxxxx"
		// first ch may conflict with cmd that has same first ch, eg. -D and -Dxxxxx

		if (ch == 'D') { _readMarco();		continue; }	// -D[name][=[value]], no need skip assign

		cmdExtract("=");	// skip assign

		if (isCmd("cwd"))		{ _readCWD();			continue; }

		if (isCmd("x"))			{ _readShaderLang();	continue; }
		if (isCmd("I"))			{ _readInclude();		continue; }
		if (isCmd("file"))		{ _readFile();			continue; }
		if (isCmd("out"))		{ _readOutput();		continue; }
		if (isCmd("profile"))	{ _readProfile();		continue; }
		if (isCmd("entry"))		{ _readEntry();			continue; }
	}

	#else
	for (;;)
	{

		if (token().isOperator("-"))	{ nextToken(); }

		//if (token().isIdentifier("hlsl") || token().isIdentifier("glsl")) { readIdentifier(req().language); continue; }

		if (token().isIdentifier("x"))			{ _readShaderLang();	continue; }
		if (token().isIdentifier("file"))		{ _readFile();			continue; }
		if (token().isIdentifier("out"))		{ _readOutput();		continue; }
		if (token().isIdentifier("profile"))	{ _readProfile();		continue; }
		if (token().isIdentifier("entry"))		{ _readEntry();			continue; }
		if (token().isIdentifier("I"))			{ _readInclude();		continue; }

		if (isFirstChar("D"))					{ _readMarco();			continue; }

		return errorUnexpectedToken();
	}

	#endif // 0

}

void ShaderCmdLineParser::_readCWD()
{
	cmdRead(info().cwd);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readCWD =========");
	SGE_DUMP_VAR(info().cwd);
	#endif // SGE_ShaderCmdLineParser_PRINT
}

#if !SGE_ShaderCmdLineParser_STREAM_IMPL
void ShaderCmdLineParser::_readShaderLang()
{
	cmdRead(req().language);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readShaderLang =========");
	SGE_DUMP_VAR(req().language);
	#endif // SGE_ShaderCmdLineParser_PRINT
}

void ShaderCmdLineParser::_readFile()
{
	cmdRead(req().inputFilename);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readFile =========");
	SGE_DUMP_VAR(req().inputFilename);
	#endif // SGE_ShaderCmdLineParser_PRINT
}

void ShaderCmdLineParser::_readOutput()
{
	cmdRead(req().outputFilename);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readOutput =========");
	SGE_DUMP_VAR(req().outputFilename);
	#endif // SGE_ShaderCmdLineParser_PRINT
}

void ShaderCmdLineParser::_readProfile()
{
	cmdRead(req().profile);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readProfile =========");
	SGE_DUMP_VAR(req().profile);
	#endif // SGE_ShaderCmdLineParser_PRINT

}

void ShaderCmdLineParser::_readEntry()
{
	cmdRead(req().entry);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readEntry =========");
	SGE_DUMP_VAR(req().entry);
	#endif // SGE_ShaderCmdLineParser_PRINT

}

void ShaderCmdLineParser::_readInclude()
{
	auto& back = req().include.dirs.emplace_back();
	cmdRead(back);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readInclude =========");
	SGE_DUMP_VAR(back);
	#endif // SGE_ShaderCmdLineParser_PRINT

}

void ShaderCmdLineParser::_readMarco()
{
	nextCmdChar();

	auto& back = req().marcos.emplace_back();

	auto& name	= back.name;
	auto& value = back.value;

	cmdExtract(name, "=");

	if (getCmdCh())
		cmdRead(value);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readMarco =========");
	SGE_DUMP_VAR(req().marcos.back().name);
	SGE_DUMP_VAR(req().marcos.back().value);
	#endif // SGE_ShaderCmdLineParser_PRINT
}

void ShaderCmdLineParser::_skipAssign()
{
	this->cmdSkipToNext("=");
}
#else
void ShaderCmdLineParser::_readShaderLang()
{
	SGE_LOG("====== _readShaderLang =========");
	_skipAssign();

	readIdentifier(req().language);

	SGE_DUMP_VAR(req().language);
}

void ShaderCmdLineParser::_readFile()
{
	SGE_LOG("====== _readFile =========");

	#if 0
	_skipAssign();
	#else
	SGE_LOG("current token: {}", token().str);

	nextToken();
	SGE_LOG("current token: {}", token().str);

	if (token().isOperator("="))
	{
		nextToken();
		SGE_LOG("current token: {}", token().str);
	}
	#endif // 0

	SGE_LOG("====== _End skip assign =========");

	SGE_LOG("cmd[2]: {}", (*_cmdArg)[2]);

	#if 0
	// file named .shader
	if (token().isOperator("."))
	{
		req().inputFilename += token().str; nextToken();
		req().inputFilename += token().str;
	}
	// file named xxxx.shader
	else
	{
		readIdentifier(req().inputFilename); nextToken();
		if (token().isOperator("."))
		{
			req().inputFilename += token().str; nextToken();
			req().inputFilename += token().str;
		}
	}
	#endif // 0

	SGE_LOG("current token: {}", token().str);

	if (!token().isString())
		_error("expected String");

	readString(req().inputFilename);
	SGE_DUMP_VAR(req().inputFilename);
}

void ShaderCmdLineParser::_readOutput()
{
	SGE_LOG("====== _readOutput =========");
	_skipAssign();

	if (!token().isString())
		_error("expected String");

	readString(req().outputFilename);
	SGE_DUMP_VAR(req().outputFilename);
}

void ShaderCmdLineParser::_readProfile()
{
	SGE_LOG("====== _readProfile =========");
	_skipAssign();

	for (;;)
	{
		if (token().isOperator("-")  || token().isNone())
			break;

		req().profile += token().str;
		nextToken();
	}

	nextToken();
	SGE_DUMP_VAR(req().profile);
}

void ShaderCmdLineParser::_readEntry()
{
	SGE_LOG("====== _readEntry =========");
	_skipAssign();

	for (;;)
	{
		if (token().isOperator("-")  || token().isNone())
			break;

		req().entry += token().str;
		nextToken();
	}

	nextToken();
	SGE_DUMP_VAR(req().entry);
}

void ShaderCmdLineParser::_readInclude()
{
	SGE_LOG("====== _readInclude =========");

	_skipAssign();

	for (;;)
	{
		if (token().isOperator("-") || token().isNone())
			break;

		if (!token().isString())
			_error("expected String");

		auto& back = req().includes.emplace_back();
		readString(back);

		SGE_DUMP_VAR(token().str);
	}

	nextToken();
	SGE_DUMP_VAR(req().includes.front());
}

void ShaderCmdLineParser::_readMarco()
{
	SGE_LOG("====== _readMarco =========");
	//_skipAssign();

	auto& back = req().marcos.emplace_back();

	extract(back.name, "D", "="); expectOperator("=");

	if (token().isOperator("-"))
	{
		SGE_DUMP_VAR(req().marcos.back().name);
		SGE_DUMP_VAR(req().marcos.back().value);
		return;
	}
	extract(back.value, "", " ");

	SGE_DUMP_VAR(req().marcos.back().name);
	SGE_DUMP_VAR(req().marcos.back().value);
}

void ShaderCmdLineParser::_skipAssign()
{
	nextToken();
	if (token().isOperator("="))
		nextToken();
}
#endif

void ShaderCmdLineParser::_skipProgramName()
{
	auto programName = nextCmd(); (void)programName;
	info().cwd = FilePath::dirname(programName);
	if (!info().cwd.size())
	{
		info().cwd = "/";
	}

	#if SGE_ShaderCmdLineParser_STREAM_IMPL
	for (size_t i = token().str.size(); i < programName.size(); i++)
	{
		nextChar();
	}
	nextToken();
	#endif // SGE_ShaderCmdLineParser_STREAM_IMPL
	nextCmd();
}

void ShaderCmdLineParser::extract(String& out, StrView target, StrView delimiter)
{
	out.clear();

	auto* beg = token().str.begin();
	auto* end = token().str.end();

	// case 1: -I../../xxx.h
	// case 2: -Ixxx.h
	for (auto& s : target)
	{
		SGE_ASSERT(s == *beg);
		beg++;
	}
	if (beg != end)
	{
		StrView tmp{ beg, size_t(end - beg)};
		out += tmp;
	}

	char ch = 0;
	for (;;)
	{
		ch = _ch;
		if (!ch || StringUtil::hasChar(delimiter, ch))
			break;

		out += ch;
		nextChar();
	}
	nextToken();
}

}