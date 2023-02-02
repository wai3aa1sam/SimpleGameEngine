#include <sge_core-pch.h>
#include "CommandLine.h"

namespace sge {

CommandLine::CommandLine(StrView cmd)
{
	execute(cmd);
}

void CommandLine::execute(StrView cmd)
{
	_shell.create("cmd", cmd);
}


}