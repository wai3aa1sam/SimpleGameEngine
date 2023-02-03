#include <sge_engine-pch.h>
#include "CompileShader.h"

namespace sge {

void CompileShader::hotReload()
{
	auto* ps = ProjectSettings::instance();

	if (!Directory::isExist(ps->shaderRecompileListPath()))
		return;

	FileStream fs;
	Vector<u8> data;

	fs.openWrite(ps->shaderRecompileListPath(), false);
	data.resize(fs.fileSize());
	fs.readBytes(data);

	auto textData = StrView_make(data);
	auto pair = StringUtil::splitByChar(textData, "\n");

	while (pair.first.size() != 0)
	{
		_hotReloadShader(pair.first);
		pair = StringUtil::splitByChar(pair.second, "\n");
	}

	fs.setFileSize(0);
}

SPtr<Shader> CompileShader::compileIfNotExist(Shader* shader, const Permutations& permuts)
{
	auto* proj	= ProjectSettings::instance();
	auto* rdr	= Renderer::instance();

	if (permuts.size() == 0)
		return rdr->createShader(shader->filename());;

	TempString shaderPath;
	TempString permutName;

	permuts.nameTo(permutName);
	FmtTo(shaderPath, "{}/{}/{}/{}", proj->importedPath(), shader->filename(), proj->shaderPermutationPath(), permutName);

	bool isExist = Directory::exists(shaderPath);
	if (isExist)
		return rdr->createShader(shader, permuts);

	SGE_LOG("--- Compile shader permutation: {}", shader->filename());
	{
		String args;
		args.reserve(512);

		TempString shaderCompilerRoot = proj->shaderCompilerRoot();

		#if SGE_OS_WINDOWS
		// /k pause
		// /c close
		args.append("/c ");		// args.append(proj->projectRoot()); args.append("&&");
		args.append(shaderCompilerRoot.c_str());
		args.append(" -cwd=");		args.append(proj->projectRoot());
		args.append(" -file=");		args.append(shader->filename());

		for (size_t i = 0; i < permuts.size(); i++)
		{
			auto& permut = permuts[i];
			args.append(" -D");		args.append(permut.name().data()); args.append("=");  args.append(permuts[i].value().data());
		}
		#else
		#error("compileIfNotExist does not support other platform cmdline")
		#endif

		CommandLine cmd = { args };
	}

	return rdr->createShader(shader, permuts);
}

void CompileShader::reloadPermutation()
{
	auto* rdr		= Renderer::instance();
	auto& refData	= rdr->shaderRefData();

	const auto& req = refData.permutationRequest();

	for (auto pair : req)
	{
		auto& mtl = pair.first;
		mtl->setShader(compileIfNotExist(mtl->_internal_shader(), mtl->permutations()));
	}

	refData.clearPermutationRequest();
}

void CompileShader::_hotReloadShader(StrView filename)
{
	auto* rdr = Renderer::instance();
	auto& srd = rdr->shaderRefData();

	auto* shader = srd.findShader(filename);
	auto* shaders = srd.getShaders(filename);

	if (!shader || !shaders)
		return;

	// remove all permutation first
	shaders->clear();
	shaders->emplace_back(shader);
	shader->reset();

	auto* materialTable = srd.getMaterials(shader);
	if (!materialTable)
		return;

	auto* info = shader->info();
	SGE_ASSERT(info, "no shader info");

	for (auto& mtl : *materialTable)
	{
		// if pervious has permutation, compare its value to new shader info, if no change
		if (info->permuts.size() == 0)
		{
			mtl->setShader(shader);
		}
		else
		{
			mtl->resetPermutation(info->permuts);
			const auto& permutations = mtl->permutations();

			auto permutShader = compileIfNotExist(shader, permutations);

			mtl->setShader(permutShader);
		}
	}
}

}