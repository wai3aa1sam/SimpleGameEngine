#include "sge_shader_compiler-pch.h"
#include "ShaderCmdLineParser.h"

#define SGE_ShaderCmdLineParser_Debug 0

#if SGE_ShaderCmdLineParser_Debug
#define SGE_ShaderCmdLineParser_PRINT 0
#endif // SGE_ShaderCmdLineParser_Debug


namespace sge {

void ShaderCmdLineParser::readArgv(Info& info, const CmdLineArg& arg)
{
	TempString cmdStream;
	arg.stream(cmdStream);

	SGE_LOG("args: {}", cmdStream);

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

	char ch = 0;
	for (;; nextCmd())
	{
		ch = getCmdCh();
		if (!ch)
			break;
		if (!cmd().size())
			break;

		if (ch != '-')
			throw SGE_ERROR("Error: {} in correct syntax, - is missing", cmd());

		nextCmdChar();
		ch = getCmdCh();

		// "-[cmd]=xxxxx"
		// first ch may conflict with cmd that has same first ch, eg. -D and -Dxxxxx

		if (ch == 'D') { _readMarco();		continue; }	// -D[name][=[value]], no need skip assign

		cmdExtract("=");	// skip assign

		if (isCmd("makeCompile"))	{ _info->isGNUMakeCompile = true; continue; }
		if (isCmd("generateMake"))	{ _info->isGenerateMake   = true; continue; }
		if (isCmd("permutName"))	{ cmdRead(req().permutName); continue; }

		if (isCmd("cwd"))		{ _readCWD();			continue; }

		if (isCmd("x"))			{ _readShaderLang();	continue; }
		if (isCmd("I"))			{ _readInclude();		continue; }
		if (isCmd("file"))		{ _readFile();			continue; }
		if (isCmd("out"))		{ _readOutput();		continue; }
		if (isCmd("profile"))	{ _readProfile();		continue; }
		if (isCmd("entry"))		{ _readEntry();			continue; }
	}
}

void ShaderCmdLineParser::_readCWD()
{
	cmdRead(info().cwd);

	#if SGE_ShaderCmdLineParser_PRINT
	SGE_LOG("====== _readCWD =========");
	SGE_DUMP_VAR(info().cwd);
	#endif // SGE_ShaderCmdLineParser_PRINT
}

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
	auto& back = req().include.emplaceBackDir();
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

void ShaderCmdLineParser::_skipProgramName()
{
	auto programName = nextCmd(); (void)programName;
	info().cwd = FilePath::dirname(programName);
	if (!info().cwd.size())
	{
		info().cwd = "/";
	}
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
		SGE_ASSERT(s == *beg); (void)s;
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