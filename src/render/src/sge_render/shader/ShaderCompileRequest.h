#pragma once

#include "ShaderInfo.h"

namespace sge {

class Shader;
class Material;

struct ShaderCompileRequest
{

};

class ShaderCompile : public NonCopyable
{
public:
	ShaderCompile(StrView filename);


protected:

	Set<SPtr<Material>> materials;

};

}