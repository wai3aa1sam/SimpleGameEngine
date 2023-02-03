#pragma once

#include "Process.h"

namespace sge {

class CommandLine : public NonCopyable
{
public:
	CommandLine() = default;
	CommandLine(StrView cmd);

	void execute(StrView cmd);

private:
	Process _proc;
};

}