#include "Shader.h"
#include <sge_render/Renderer.h>

namespace sge {

Shader::Shader(StrView filename) 
	: _filename(filename)
{
	auto* proj = ProjectSettings::instance();
	auto infoFilename = Fmt("{}/{}/info.json", proj->importedPath(), filename);
	JsonUtil::readFile(infoFilename, _info);
}

Shader::Shader(StrView filename, const Permutations& permuts)
	: Shader(filename)
{
	_shadername = filename;
	permuts.nameTo(_shadername);
	_permuts = permuts;

	//compileIfNoExist(this, permuts);
}

Shader::~Shader() {
	auto* renderer = Renderer::instance();
	renderer->onShaderDestory(this);
}

void Shader::reset()
{
	onReset();
}

ShaderPass::ShaderPass(Shader* shader, ShaderInfo::Pass& info) 
	: _shader(shader)
	, _info(&info)
{

}


}

