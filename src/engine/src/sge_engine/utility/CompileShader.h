#pragma once

namespace sge {

class CompileShader : public NonCopyable
{
public:
	using Permutations = Shader::Permutations;

	static void hotReload();

	static SPtr<Shader> compileIfNotExist(Shader* shader, const Permutations& permuts);

	static void reloadPermutation();

private:
	static void _hotReloadShader(StrView filename);

};

}