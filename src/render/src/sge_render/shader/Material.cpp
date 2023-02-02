#include "Material.h"
#include <sge_render/Renderer.h>

namespace sge {

void Material::setShader(Shader* shader) {
	//if (_shader == shader) return;
	if (!shader)
		return;
	_shader = shader;
	_passes.clear();
	_passes.reserve(shader->passes().size());
	for (auto& shaderPass : shader->passes()) {
		UPtr<Pass> pass = onCreatePass(shaderPass.get());
		_passes.emplace_back(std::move(pass));
	}

	//_permuts.clear();
	resetPermutation(shader->info()->permuts);

	onSetShader();

	Renderer::instance()->shaderRefData().setMaterialShader(this, shader);
}

void Material::setPermutation(StrView name, StrView value)						
{
	if (_permuts.size() == 0)
	{
		resetPermutation(_shader->info()->permuts);
	}
	_setPermutation(name, value); 
	Renderer::instance()->shaderRefData().sendPermutationRequest(this); 
}

void Material::resetPermutation(const ShaderInfo::Permutations& permutations)	{ _permuts.create(permutations); }
void Material::clearPermutation()
{
	_permuts.clear();
	auto shader = Renderer::instance()->createShader(_shader->filename());
	_shader = shader;
}

MaterialPass_Stage::MaterialPass_Stage(MaterialPass* pass, ShaderStage* shaderStage) 
	: _pass(pass)
	, _shaderStage(shaderStage)
{
	auto* info = shaderStage->info();
	{
		auto cbCount = info->constBuffers.size();
		_constBuffers.resize(cbCount);

		for (size_t i = 0; i < cbCount; i++) {
			auto& cb = _constBuffers[i];
			cb.create(info->constBuffers[i]);
		}
	}

	{
		auto texCount = info->textures.size();
		_texParams.resize(texCount);

		for (size_t i = 0; i < texCount; i++) {
			auto& t = _texParams[i];
			t.create(info->textures[i]);
		}
	}
}

void MaterialPass_Stage::ConstBuffer::create(const Info& info) {
	_info = &info;
	_gpuDirty = true;

	cpuBuffer.resize(info.dataSize);

	RenderGpuBuffer::CreateDesc desc;
	desc.type = RenderGpuBufferType::Const;
	desc.bufferSize = info.dataSize;

	gpuBuffer = Renderer::instance()->createGpuBuffer(desc);
}


void MaterialPass_Stage::ConstBuffer::uploadToGpu() {
	if (!_gpuDirty) return;
	_gpuDirty = false;

	if (!gpuBuffer) return;
	gpuBuffer->uploadToGpu(cpuBuffer);
}

void MaterialPass_Stage::ConstBuffer::errorType() {
	throw SGE_ERROR("ConstBuffer setParam type mismatch");
}

Texture* MaterialPass_Stage::TexParam::getUpdatedTexture() {
	if (!_tex) {
		auto* renderer = Renderer::instance();
		switch (_info->dataType) {
			case DataType::Texture2D: return renderer->stockTextures.error; break;
			default: throw SGE_ERROR("unsupported texture type");
		}
	}

	// TODO update texture
	return _tex;
}



}
