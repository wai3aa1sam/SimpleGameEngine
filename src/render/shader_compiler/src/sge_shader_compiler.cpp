#include "ShaderCompiler_DX11.h"
#include "ShaderParser.h"
#include "ShaderCompiler.h"
#include <sge_core/string/CmdLineParser.h>

namespace sge {

//Names s_names;

}

int main(int argc, char** argv) {

	sge::ShaderCompiler app;
	
#if 0
	char* argvs[]
	{
		"sge_shader_compiler.exe",
		"-cwd=../../../../../examples/Test101",
		"-x=hlsl",
		//"-file = \"Assets\\\\Shaders\\\\test.shader\"",
		//"-file=\"../Test101/Assets/Shaders/test.shader\"", // legacy
		"-file=Assets/Shaders/test.shader",

		"-out=dx11/pass0/.bin",
		"-profile=vs_5_0",
		"-entry=vs_main",

		"-I=../../abc/v",
		"-I=../../abc/c /c../..",
		"-I=../../built-in",

		"-DSGE_IS_INVERT_Y=",
		"-D_JJHHY=5584",
		"-I=../../built-in/shader",
		"-D_Hahahah",

		"-generateMake",
		"-makeCompile"

		//"-I=\"../../../built-in\"",
	};

	app.parseCmdLine(sge::CmdLineArg((int)std::extent<decltype(argvs)>(), argvs));

#else
	app.parseCmdLine(sge::CmdLineArg(argc, argv));
#endif // 0


	app.run();

	return 0;
}