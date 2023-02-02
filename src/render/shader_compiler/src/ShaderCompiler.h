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

	virtual void onRun() 
	{
		auto* proj = ProjectSettings::instance();
		{
			String file = getExecutableFilename();
			String path = FilePath::dirname(file);

			path.append("/../../../../../..");
			proj->setSgeRoot(path);

			proj->setProjectRoot(compileInfo.cwd);

			compileInfo.sgeRoot = proj->sgeRoot().data();
		}

		{
			auto& include = compileInfo.comileRequest.include;
			include.emplaceBackDir() = proj->buildInRoot().c_str();
			include.emplaceBackDir() = proj->buildInShaderRoot().c_str();
		}

		auto& shaderFilename = compileInfo.comileRequest.inputFilename;
		compile(shaderFilename);
	}

	bool isGNUMakeCompile() 
	{
		return compileInfo.isGNUMakeCompile;
	}

	bool isGenerateMake() 
	{
		return compileInfo.isGenerateMake;
	}

	bool permutNameTo(String& o)
	{
		const ShaderInfo& info	= compileInfo.shaderInfo;
		const auto& cReq		= compileInfo.comileRequest;
		
		if (info.permuts.size() == 0)
			return false;

		TempString tmp;
		//tmp += "_";
		for (size_t i = 0; i < info.permuts.size(); i++)
		{
			const auto& permut = info.permuts[i];
			int valueIdx = -1;
			const CompileRequest::Marco* targetMar = nullptr;
			for (auto& mar : cReq.marcos)
			{
				if (mar.name == permut.name)
				{
					targetMar = &mar;
					break;
				}
			}
			if (!targetMar)
				return false;
			valueIdx = permut.findValueIdx(targetMar->value);
			if (valueIdx == -1)
				return false;
			tmp += StringUtil::toString(valueIdx);
		}

		o = tmp;
		return true;
	}
	void permutationCompile(String& outputPath)
	{
		auto* ps = ProjectSettings::instance();
		auto& cReq = compileInfo.comileRequest;
		if (cReq.marcos.size() > 0)
		{
			if (cReq.permutName.size() == 0)
				permutNameTo(cReq.permutName);
			if (cReq.permutName.size() == 0)
				return;
			outputPath += Fmt("/{}/{}", ps->shaderPermutationPath(), cReq.permutName);
			Directory::create(outputPath);
		}
	}

	void compile(StrView shaderFilename)
	{
		auto* ps = ProjectSettings::instance();

		String outputPath;
		outputPath.reserve(512);
		outputPath = Fmt("{}/{}", ps->importedPath(), shaderFilename);

		Directory::create(outputPath);
		
		// if make is triggered, the old permutation files must be invalid
		if (isGNUMakeCompile())
		{
			TempString permutPath = outputPath.c_str(); permutPath += "/";
			permutPath += ps->shaderPermutationPath();
			Directory::removeAll(permutPath);
		}

		ShaderInfo& info = compileInfo.shaderInfo;
		{
			ShaderParser parser(compileInfo);
			parser.readFile(info, shaderFilename);

			auto jsonFilename = Fmt("{}/info.json", outputPath);
			JsonUtil::writeFileIfChanged(jsonFilename, info, false);
		}

		permutationCompile(outputPath);

		create_bin_path(info, outputPath);

		if (isGNUMakeCompile())
		{
			// spirv will compile from make
			//compile_dx11_bin(shaderFilename, outputPath);
			compile_all_dx11(shaderFilename, outputPath);
			markToReload();
		}
		else
		{
			compile_all_dx11(shaderFilename, outputPath);
			compile_all_spirv(shaderFilename, outputPath);
		}

		if (isGenerateMake())
			ShaderGNUMake::generate(compileInfo);

		SGE_LOG("---- end ----");
	}

	void compile_dx11_bin(StrView shaderFilename, StrView outputPath)
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

	void compile_all_spirv(StrView shaderFilename, StrView outputPath)
	{
		ShaderInfo& info = compileInfo.shaderInfo;
		const char* apiName = "spirv";

		size_t passIndex = 0;
		for (auto& pass : info.passes) {
			auto passOutPath = Fmt("{}/{}/pass{}", outputPath, apiName, passIndex);

			if (pass.vsFunc.size()) 
			{
				compile_spirv(passOutPath, ShaderStageMask::Vertex, shaderFilename, pass.vsFunc);
			}

			if (pass.psFunc.size()) 
			{
				compile_spirv(passOutPath, ShaderStageMask::Pixel, shaderFilename, pass.psFunc);
			}

			passIndex++;
		}
	}

	void compile_spirv(StrView outPath, ShaderStageMask shaderStage, StrView srcFilename, StrView entryFunc)
	{
		auto* proj = ProjectSettings::instance();
		auto& cReq = compileInfo.comileRequest;
		//TempString shaderCompilerRoot = proj->shaderCompilerRoot();

		String args; args.reserve(512);
		const char* stageProfile	= getStageProfile(shaderStage, ApiType::Vulkan);
		const char* profile			= getGlslcStageProfile(shaderStage);
		auto binName				= Fmt("{}.bin", stageProfile);

		args.append("/c cd ");				args.append(proj->projectRoot()); args.append(" &&");
		args.append(" glslc");
		args.append(" -x ");				args.append("hlsl");
		args.append(" -fshader-stage=");	args.append(profile);
		args.append(" -fentry-point=");		args.append(entryFunc.data());
		args.append(" -MD ");				args.append(srcFilename.data());
		args.append(" -o ");				args.append(outPath.data()); args.append("/"); args.append(binName.c_str());

		for (auto& inc : cReq.include.dirs())
		{
			args.append(" -I"); args.append("\""); args.append(inc); args.append("\"");
		}

		for (auto& mar : cReq.marcos)
		{
			args.append(" -D"); args.append("\""); args.append(mar.name); args.append("="); args.append(mar.value); args.append("\"");
		}

		//SGE_LOG("--- glslc args {}", args);

		CommandLine cmd = { args };
	}

	void create_bin_path(const ShaderInfo& info, StrView outputPath)
	{
		using ApiType = ShaderGNUMake::ApiType;

		const char* apiPath = getBuildApiPath(ApiType::Vulkan);
		TempString passOutPath;

		size_t passIndex = 0;
		for (auto& pass : info.passes) {
			(void) pass;
			passOutPath.clear();
			FmtTo(passOutPath, "{}/{}/pass{}", outputPath, apiPath, passIndex);
			Directory::create(passOutPath);
		}
	}

	void markToReload()
	{
		auto* proj = ProjectSettings::instance();

		FileStream fs;
		fs.openWrite(proj->shaderRecompileListPath(), false);

		TempString tmp;

		//tmp += "\"";

		auto pair = StringUtil::splitByChar(compileInfo.comileRequest.inputFilename, "\\");
		while (pair.first.size() != 0)
		{
			tmp += pair.first;
			if (pair.second.size() != 0)
				tmp += "/";
			pair = StringUtil::splitByChar(pair.second, "\\");
		}
		//tmp += "\"";
		tmp += "\n";
		fs.setPos(fs.fileSize());
		fs.writeBytes(ByteSpan_make(tmp));
	}

	CompileInfo compileInfo;
};


}