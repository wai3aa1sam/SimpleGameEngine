#include "ProjectSettings.h"
#include <sge_core/file/Directory.h>
#include <sge_core/file/FilePath.h>

#include <sge_core/log/Log.h>

namespace sge {

ProjectSettings* ProjectSettings::instance() {
	static ProjectSettings s;
	return &s;
}

void ProjectSettings::setSgeRoot(StrView path)
{
	_sgeRoot = FilePath::realpath(path);
}

void ProjectSettings::setProjectRoot(StrView path) {

	_projectRoot = FilePath::realpath(path);
	Directory::setCurrent(path);

	//auto dir = Directory::getCurrent();
	//SGE_LOG("projectRoot = {}", dir);
}

StrView ProjectSettings::importedPath() const			{ return "LocalTemp/Imported"; }
StrView ProjectSettings::buildInPath() const			{ return "built-in"; }
StrView ProjectSettings::buildInShaderPath() const		{ return "built-in/shader"; }
StrView ProjectSettings::shaderPermutationPath() const	{ return "permutation"; }
StrView ProjectSettings::shaderCompilerPath() const		
{
	return "build/SimpleGameEngine-x64-windows/src/render/shader_compiler/" SGE_BUILD_CONFIG_STR "/sge_shader_compiler.exe";
}

StrView ProjectSettings::shaderRecompileListPath() const { return "LocalTemp/Imported/shader_recompile_list.txt"; }


const String& ProjectSettings::sgeRoot() const
{
	return _sgeRoot;
}

const String& ProjectSettings::shaderCompilerRoot() const
{
	static CallOnce co{ 
		[&]() 
		{ 
			toRoot(constCast(_shaderCompilerRoot), sgeRoot(), shaderCompilerPath());
		} 
	};
	return _shaderCompilerRoot;
}

const String& ProjectSettings::buildInRoot() const
{
	static CallOnce co{ 
		[&]() 
		{ 
			toRoot(constCast(_buildInRoot), sgeRoot(), buildInPath());
		} 
	};
	return _buildInRoot;
}

const String& ProjectSettings::buildInShaderRoot() const
{
	static CallOnce co{ 
		[&]() 
		{ 
			toRoot(constCast(_buildInShaderRoot), sgeRoot(), buildInShaderPath());
		} 
	};
	return _buildInShaderRoot;
}

void ProjectSettings::toRoot(String& dst, const StrView root, const StrView path)
{
	dst += root;
	dst += "/";
	dst += path;
}

}