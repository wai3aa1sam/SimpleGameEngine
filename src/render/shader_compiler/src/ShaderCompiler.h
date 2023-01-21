#pragma once
#include "ShaderGNUMake.h"
#include "ShaderCmdLineParser.h"

namespace sge {

class ShaderCompiler : public ConsoleApp {
public:

	void parseCmdLine(const CmdLineArg& cmdArg) 
	{
		ShaderCmdLineParser parser;
		parser.readArgv(compileInfo, cmdArg);
	}

protected:

	virtual void onRun() {
		{
			String file = getExecutableFilename();
			String path = FilePath::dirname(file);
			StrView dir = path;

			SGE_LOG("cwd: {}", compileInfo.cwd);
			if (FilePath::isRealpath(compileInfo.cwd))
			{
				path = compileInfo.cwd;
			}
			else
			{
				path.append("/"); path.append(compileInfo.cwd);
				#if 0
				String projectRoot = "/../../../../../../examples/Test101";
				//compileInfo.projectRoot = "/../../../../../../examples/Test101";
				path.append(projectRoot);
				#else
				//path.append("/../../../../../../examples/Test101");
				#endif // 0
			}

			Directory::setCurrent(path);
			auto* proj = ProjectSettings::instance();
			proj->setProjectRoot(path);

			compileInfo.sgeRoot = dir;
			compileInfo.sgeRoot.append("/../../../../../..");
		}

		#if 0
		auto& names	  = ShaderGNUMake::s_names;	(void) names;

		compileInfo.buildConfig = "Debug";

		//compileInfo.sgeRoot				= "../../../../../../..";
		//compileInfo.projectRoot			= Fmt("$({})/examples/Test101/Assets/Shaders", names.projectRoot);
		compileInfo.compilerMakePath	= Fmt("$({})/makeFile", names.sgeRoot);
		compileInfo.projectMakePath		= Fmt("$({})/makeFile", names.projectRoot);

		compileInfo.compilerPath		= Fmt("build/SimpleGameEngine-x64-windows/src/render/shader_compiler/{}/sge_shader_compiler.exe", compileInfo.buildConfig);

		compileInfo.shaderCompiler		= Fmt("$({})/$({})", names.sgeRoot, names.shaderCompilerPath);
		compileInfo.builtInPath			= Fmt("$({})/built-in", names.sgeRoot);

		//compileInfo.comileRequest.includes.emplace_back(Fmt("$({})/built-in", names.sgeRoot));
		#endif // 0

		#if 0
		compileInfo.comileRequest.inputFilename = "Assets/Shaders/test.shader";
		SGE_LOG("shaderFilename: {}", FilePath::getRealPath(compileInfo.comileRequest.inputFilename));
		#endif // 1

		auto& shaderFilename = compileInfo.comileRequest.inputFilename;
		compile(shaderFilename);
	}

	bool isDefaultCompile() 
	{
		auto& req = compileInfo.comileRequest;
		bool isDefault = ! ((req.entry.size() > 0) && (req.profile.size() > 0) && (req.inputFilename.size() > 0));
		return isDefault;
	}

	void compile(StrView shaderFilename)
	{
		String outputPath = Fmt("LocalTemp/Imported/{}", shaderFilename);
		Directory::create(outputPath);

		ShaderInfo& info = compileInfo.shaderInfo;
		{
			ShaderParser parser(compileInfo);
			parser.readFile(info, shaderFilename);

			auto jsonFilename = Fmt("{}/info.json", outputPath);
			JsonUtil::writeFileIfChanged(jsonFilename, info, false);
		}

		create_bin_path(info, outputPath);

		if (isDefaultCompile())
		{
			compile_all_dx11(shaderFilename, outputPath);
		}
		else
		{
			compile_bin(shaderFilename, outputPath);
		}

		ShaderGNUMake::generate(compileInfo);

		SGE_LOG("---- end ----");
	}

	void compile_bin(StrView shaderFilename, StrView outputPath)
	{
		auto& req = compileInfo.comileRequest;

		auto& entry = req.entry;

		auto outPath = Fmt("{}/{}", outputPath, FilePath::dirname(req.outputFilename));
		ShaderStageMask mask; parseShaderStageMask(mask, req.profile);

		if (entry.size()) {

			ShaderCompiler_DX11 c(compileInfo);
			c.compile(outPath, mask, shaderFilename, entry);
		}
	}

	void compile_all_dx11(StrView shaderFilename, StrView outputPath)
	{
		ShaderInfo& info = compileInfo.shaderInfo;

		size_t passIndex = 0;
		for (auto& pass : info.passes) {
			auto passOutPath = Fmt("{}/dx11/pass{}", outputPath, passIndex);

			if (pass.vsFunc.size()) {
				ShaderCompiler_DX11 c(compileInfo);
				c.compile(passOutPath, ShaderStageMask::Vertex, shaderFilename, pass.vsFunc);
			}

			if (pass.psFunc.size()) {
				ShaderCompiler_DX11 c(compileInfo);
				c.compile(passOutPath, ShaderStageMask::Pixel, shaderFilename, pass.psFunc);
			}

			passIndex++;
		}
	}

	void create_bin_path(const ShaderInfo& info, StrView outputPath)
	{
		using ApiType = ShaderGNUMake::ApiType;
		const char* apiPath = ShaderGNUMake::getBuildApiPath(ApiType::Vulkan);
		TempString passOutPath;

		size_t passIndex = 0;
		for (auto& pass : info.passes) {
			(void) pass;
			passOutPath.clear();
			FmtTo(passOutPath, "{}/{}/pass{}", outputPath, apiPath, passIndex);
			Directory::create(passOutPath);
		}
	}

	CompileInfo compileInfo;
};


}