#pragma once

#include <sge_render.h>
#include <sge_render/backend/dx11/Render_DX11_Common.h>
#include "CompileRequest.h"

namespace sge {

using DX11_ID3DInclude			= ID3DInclude;
using DX11_D3DShaderMarco		= D3D_SHADER_MACRO;

class ShaderCompiler_DX11 : public NonCopyable {
	using Util = DX11Util;
	using DataType = RenderDataType;
public:
	ShaderCompiler_DX11(CompileInfo& cInfo);
	void compile(StrView outFilename, ShaderStageMask shaderStage, StrView srcFilename, StrView entryFunc);

private:
	void _reflect(StrView outFilename, ByteSpan bytecode, ShaderStageMask stage, StrView profile);
	void _reflect_inputs		(ShaderStageInfo& outInfo, ID3D11ShaderReflection* reflect, D3D11_SHADER_DESC& desc);
	void _reflect_constBuffers	(ShaderStageInfo& outInfo, ID3D11ShaderReflection* reflect, D3D11_SHADER_DESC& desc);
	void _reflect_textures		(ShaderStageInfo& outInfo, ID3D11ShaderReflection* reflect, D3D11_SHADER_DESC& desc);
	void _reflect_samplers		(ShaderStageInfo& outInfo, ID3D11ShaderReflection* reflect, D3D11_SHADER_DESC& desc);

	void _init_marco(Vector<DX11_D3DShaderMarco>& sh_mar);

	CompileInfo* _cInfo = nullptr;
};

}