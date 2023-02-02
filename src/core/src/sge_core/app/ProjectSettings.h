#pragma once

#include <sge_core/base/Error.h>

namespace sge {

class CallOnce
{
public:
	template<class Callable, class... Args>
	CallOnce(Callable&& f, Args&&... args)
	{
		std::call_once(_flag, f, std::forward<Args>(args)...);
	}

private:
	std::once_flag _flag;
};

struct ProjectSettings : public NonCopyable {

	static ProjectSettings* instance();

	void setSgeRoot(StrView path);
	void setProjectRoot(StrView path);

	StrView importedPath()	const;
	StrView buildInPath() const;
	StrView buildInShaderPath() const;
	StrView shaderPermutationPath() const;
	StrView shaderCompilerPath() const;
	
	StrView shaderRecompileListPath() const;


	const String& projectRoot()	const { return _projectRoot; }
	const String& sgeRoot() const;
	const String& shaderCompilerRoot() const;
	const String& buildInRoot() const;
	const String& buildInShaderRoot() const;

private:
	static void toRoot(String& dst, const StrView root, const StrView path);

private:
	String _projectRoot;

	String _sgeRoot;

	String _shaderCompilerRoot;

	String _buildInRoot;
	String _buildInShaderRoot;


};

}