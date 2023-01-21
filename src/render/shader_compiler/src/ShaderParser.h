#pragma once
#include <sge_core/string/Lexer.h>
#include <sge_render/shader/ShaderInfo.h>
#include "CompileRequest.h"

namespace sge {

class ShaderParser : public Lexer
{
public:
	ShaderParser(CompileInfo& cInfo);
	void readFile(ShaderInfo& outInfo_, StrView filename_);
	void readMem(ShaderInfo& outInfo_, ByteSpan data_, StrView filename_);

private:
	void _readShader();
	void _readProperties();
	void _readProperty();
	void _readPass();

	void _readInclude();
	void _readMarco();
	void _readValue(String& str);

	void _readBlendFunc(RenderState::BlendFunc& v);

	template<class E> void _readEnum(E& v_);

	MemMapFile _memMapFile;
	ShaderInfo* _pOutInfo = nullptr;
	CompileInfo* _cInfo = nullptr;
};

#if 0
#pragma mark TokenType_Impl
#endif // 0
#if 1    // ShaderParser_Impl

template<class E> 
void ShaderParser::_readEnum(E& v_)
{
	if (!_token.isIdentifier()) {
		errorUnexpectedToken();
		return;
	}

	if (!enumTryParse(v_, _token.str)) {
		error("read enum [{}]", _token.str);
		return;
	}
	nextToken();
}


#endif
}