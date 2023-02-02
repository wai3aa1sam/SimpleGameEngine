#include "ShaderParser.h"

namespace sge {

ShaderParser::ShaderParser(CompileInfo& cInfo)
	:
	_cInfo(&cInfo)
{
}

void ShaderParser::readFile(ShaderInfo& outInfo_, StrView filename_)
{
	_memMapFile.open(filename_);
	readMem(outInfo_, _memMapFile, filename_);
}

void ShaderParser::readMem(ShaderInfo& outInfo_, ByteSpan data_, StrView filename_)
{
	outInfo_.clear();
	_pOutInfo = &outInfo_;
	reset(data_, filename_);
	skipNewlineTokens();

	while (!_token.isNone() && !_token.isIdentifier("Shader"))
	{
		nextToken();
	}

	if (_token.isIdentifier("Shader")) {
		_readShader();
	}
	else {
		error("missing Shader tag");
	}
}

void ShaderParser::_readShader()
{
	nextToken();
	expectOperator("{");

	for (;;) {
		if (_token.isOperator("}"))		{ nextToken(); break; }
		if (_token.isNewline())			{ nextToken(); continue; }

		if (_token.isIdentifier("Properties"))	{ _readProperties();	continue; }
		if (_token.isIdentifier("Pass"))		{ _readPass();			continue; }
		if (_token.isIdentifier("Permutation"))	{ _readPermutation();	continue; }

		return errorUnexpectedToken();
	}
}

void ShaderParser::_readProperties()
{
	nextToken();
	expectOperator("{");

	for (;;) {
		skipNewlineTokens();
		if (_token.isOperator("}")) { nextToken(); break; }
		_readProperty();
	}
}

void ShaderParser::_readProperty()
{
	auto& prop = _pOutInfo->props.emplace_back();

	if (_token.isOperator("["))
	{
		nextToken();
		while (!_token.isNone())
		{
			skipNewlineTokens();

			if (_token.isIdentifier("DisplayName"))
			{
				nextToken();
				expectOperator("=");
				readString(prop.displayName);
			}
			if (_token.isOperator("]")) { nextToken(); break; }

			expectOperator(",");
		}
	}

	skipNewlineTokens();

	{
		// prop type
		_readEnum(prop.propType);

		// prop name
		readIdentifier(prop.name);
	}

	// optional defaultValue
	if (_token.isOperator("=")) {
		nextToken();
		while (!_token.isNone()) {
			if (_token.isNewline()) { break; }
			prop.defaultValue += _token.str;
			nextToken();
		}
	}

	if (!_token.isNewline()) {
		errorUnexpectedToken();
	}
	nextToken();
}

void ShaderParser::_readPass()
{
	nextToken();
	auto& o = _pOutInfo->passes.emplace_back();

	if (_token.isString()) {
		readString(o.name);
	}
	expectOperator("{");

	for (;;) {
		if (_token.isOperator("}")) { nextToken(); break; }
		if (_token.isNewline())		{ nextToken(); continue; }

		if (_token.isIdentifier("VsFunc")) { nextToken(); readIdentifier(o.vsFunc); continue; }
		if (_token.isIdentifier("PsFunc")) { nextToken(); readIdentifier(o.psFunc); continue; }

		if (_token.isIdentifier("Cull")) { nextToken(); readEnum(o.renderState.cull); continue; }

		if (_token.isIdentifier("DepthTest") ) { nextToken(); readEnum(o.renderState.depthTest.op); continue; }
		if (_token.isIdentifier("DepthWrite")) { nextToken(); readBool(o.renderState.depthTest.writeMask); continue; }

		if (_token.isIdentifier("Wireframe")) { nextToken(); readBool(o.renderState.wireframe); continue; }

		if (_token.isIdentifier("BlendRGB")   ) { nextToken(); _readBlendFunc(o.renderState.blend.rgb); continue; }
		if (_token.isIdentifier("BlendAlpha") ) { nextToken(); _readBlendFunc(o.renderState.blend.alpha); continue; }

		return errorUnexpectedToken();
	}
}

void ShaderParser::_readBlendFunc(RenderState::BlendFunc& v) {
	readEnum(v.op);
	readEnum(v.srcFactor);
	readEnum(v.dstFactor);
}

void ShaderParser::_readPermutation()
{
	nextToken();
	skipNewlineTokens();
	expectOperator("{");

	for (;;) {
		skipNewlineTokens();
		if (_token.isOperator("}")) { nextToken(); break; }


		if (!_token.isIdentifier())
			errorUnexpectedToken();

		auto& permut = _pOutInfo->permuts.emplace_back();
		readIdentifier(permut.name);
		_readPermutation_Value();
		//_readValue(permut.value);
	}

}

void ShaderParser::_readPermutation_Value()
{
	if (_token.isOperator("=")) {
		nextToken();
		expectOperator("{");

		while (!_token.isNone()) {
			if (_token.isOperator("}")) { nextToken(); break; }
			if (_token.isNewline())		{ break; }

			auto& values = _pOutInfo->permuts.back().values;

			skipNewlineTokens();

			auto& value = values.emplace_back();
			value += _token.str;
			nextToken();
			if (_token.isIdentifier("f"))
				nextToken();

			if (_token.isOperator(","))
			{
				nextToken();
				continue;
			}
		}
	}
}

}