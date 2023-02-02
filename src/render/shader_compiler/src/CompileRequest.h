#pragma once
#include <sge_render/backend/dx11/Render_DX11_Common.h>
#include <sge_render/shader/ShaderInfo.h>

namespace sge {

struct CompileRequest
{
	String language;
	String inputFilename;
	String outputFilename;
	String permutName;

	String profile;
	String entry;

	struct Include 
	{ 
		void reserve()				{ _files.reserve(fileCount()); }
		void addFileCount()			{ _fileCount++; };
		int  fileCount()	const	{ return _fileCount; }

		String& emplaceBackDir()	{ return _dirs.emplace_back(); }

		String& emplaceBackFile()	{ return _files.emplace_back(); }
		void	popBackFile()		{ return _files.pop_back(); }

		void clearFiles()			{ _files.clear(); }

				String& lastFile()			{ return _files.back(); }
		const	String& lastFile()	const	{ return _files.back(); }


		Span<String>		files()			{ return _files; }
		Span<const String>	files() const	{ return _files; }

		Span<String>		dirs()			{ return _dirs; }
		Span<const String>	dirs() const	{ return _dirs; }
		
	private:
		int _fileCount = 0;
		Vector<String>		_files;
		Vector<String, 4>	_dirs;
	};
	struct Marco { String name; String value; };

	Include				include;
	Vector<Marco, 12>	marcos;
};

struct CompileInfo
{
	CompileInfo()
		: isGNUMakeCompile(false), isGenerateMake(false)
	{}
	ShaderInfo shaderInfo;

	String cwd;

	String sgeRoot;
	//String projectRoot;
	String compilerPath;

	String shaderCompiler;

	String compilerMakePath;
	String projectMakePath;

	String outputPath;

	String builtInPath;

	String buildConfig;
	
	bool   isGNUMakeCompile : 1;
	bool   isGenerateMake	: 1;

	CompileRequest comileRequest;
};


class ShaderInclude : public NonCopyable
{
public:
	using Include = CompileRequest::Include;

	ShaderInclude(CompileInfo& cInfo) : _cInfo(&cInfo) { req().include.reserve(); }
	//void setCompileInfo(CompileInfo& cInfo) { _cInfo = &cInfo; }

	bool resolve(StrView inc_path)
	{
		auto& include = req().include;
		const auto& shader_path = req().inputFilename;
		return s_resolve(include, shader_path, inc_path);
	}

	static bool s_resolve(Include& include, StrView shader_path, StrView inc_path)
	{
		auto& back = include.emplaceBackFile();
		auto is_valid = s_resolve(back, include, shader_path, inc_path);
		if (!is_valid)
		{
			include.popBackFile();
		}
		return is_valid;
	}
	
	template<class STRING>
	static bool s_resolve(STRING& out, const Include& include, StrView shader_path, StrView inc_path)
	{
		out.clear();

		const auto& dirs  = include.dirs();

		TempString tmp;
		FmtTo(tmp, "{}/{}", shader_path, inc_path);

		bool is_path_valid = Path::isFile(tmp);
		if (!is_path_valid)
		{
			for (auto& dir : dirs)
			{
				tmp.clear();
				FmtTo(tmp, "{}/{}", dir, inc_path);
				is_path_valid = Path::isFile(tmp);
				if (is_path_valid)
					break;
			}
		}

		if (is_path_valid)
			out.assign(tmp.c_str());

		return is_path_valid;
	}

	const String& lastFile() const { return req().include.lastFile(); }

		  CompileRequest& req()			{ SGE_ASSERT(_cInfo); return _cInfo->comileRequest; }
	const CompileRequest& req() const	{ SGE_ASSERT(_cInfo); return _cInfo->comileRequest; }

private:
	CompileInfo* _cInfo = nullptr;
};

class ShaderMarco : public NonCopyable
{
public:

private:

};


struct VulkanUtil
{
	VulkanUtil() = delete;

	static const char* getVkStageProfile(ShaderStageMask s)
	{
		switch (s) {
			case ShaderStageMask::Vertex:	{ return "vs_1_1"; }
			case ShaderStageMask::Pixel:	{ return "ps_1_1"; }
		default: return "";
		}
	}

};

using ApiType = Renderer::ApiType;

struct Names
{
	const char* buildDX11		= "BUILD_DX11";
	const char* buildDX12		= "BUILD_DX12";
	const char* buildOpengl		= "BUILD_OPENGL";
	const char* buildSpirv		= "BUILD_SPIRV";
	const char* buildMetal		= "BUILD_METAL";

	const char* dx11			= "DX11";
	const char* dx12			= "DX12";
	const char* opengl			= "OPENGL";
	const char* spirv			= "SPIRV";
	const char* metal			= "METAL";

	const char* currentMakeFile		= "CURRENT_MAKEFILE";
	const char* sgeRoot				= "SGE_ROOT";
	const char* projectRoot			= "PROJECT_ROOT";
	const char* shaderCompilerPath  = "SGE_SHADER_COMPILER_PATH";

	const char* shaderCompiler		= "sgeShaderCompiler";
	const char* glslc				= "glslc";

	const char* shaderFilePath		= "SHADER_FILE_PATH";
	const char* shaderFileRoot		= "SHADER_FILE_ROOT";
	const char* shaderBinRoot		= "SHADER_BIN_ROOT";

	const char* builtInPath			= "BUILT_IN_PATH";
	const char* builtInRoot			= "BUILT_IN_ROOT";
	const char* builtInShaderPath	= "BUILT_IN_SHADER_PATH";
	const char* builtInShaderRoot	= "BUILT_IN_SHADER_ROOT";

private:
	TempString tmp;
};
inline Names s_names;

static inline const char* getBuildTargetName(ApiType type)
{
	using SRC = ApiType;
	switch (type)
	{
	case SRC::DX11:		{ return s_names.buildDX11; }
	case SRC::DX12:		{ return s_names.buildDX12; }
	case SRC::OpenGL:	{ return s_names.buildOpengl; }
	case SRC::Vulkan:	{ return s_names.buildSpirv; }
	case SRC::Metal:	{ return s_names.buildMetal; }
	default:
		throw SGE_ERROR("unknow api type");
	}
}

static inline const char* getBuildBinName(ApiType type)
{
	using SRC = ApiType;
	switch (type)
	{
	case SRC::DX11:		{ return s_names.dx11; }
	case SRC::DX12:		{ return s_names.dx12; }
	case SRC::OpenGL:	{ return s_names.opengl; }
	case SRC::Vulkan:	{ return s_names.spirv; }
	case SRC::Metal:	{ return s_names.metal; }
	default:
		throw SGE_ERROR("unknow api type");
	}
}

static inline const char* getCompilerName(ApiType type)
{
	using SRC = ApiType;
	switch (type)
	{
	case SRC::DX11:		{ return s_names.shaderCompiler; }
				  //case SRC::DX12:		{ return s_names.dx12; }
				  //case SRC::OpenGL:	{ return s_names.opengl; }
	case SRC::Vulkan:	{ return nullptr; }
					//case SRC::Metal:	{ return s_names.metal; }
	default:
		throw SGE_ERROR("unknow api type");
	}
}

static inline const char* getStageProfile(ShaderStageMask mask, ApiType type)
{
	using SRC = ApiType;
	switch (type)
	{
	case SRC::DX11:		{ return DX11Util::getDxStageProfile(mask); }
				  //case SRC::DX12:		{ return s_names.dx12; }
				  //case SRC::OpenGL:	{ return s_names.opengl; }
	case SRC::Vulkan:	{ return VulkanUtil::getVkStageProfile(mask); }
					//case SRC::Metal:	{ return s_names.metal; }
	default:
		throw SGE_ERROR("unknow api type, getBuildBinName()");
	}
}

static inline const char* getGlslcStageProfile(ShaderStageMask mask)
{
	using SRC = ShaderStageMask;
	switch (mask)
	{
	case SRC::Vertex:	{ return "vertex"; }
	case SRC::Pixel:	{ return "fragment"; }
	default:
		throw SGE_ERROR("unknow ShaderStageMask, getGlslcStageProfile()");
	}
}

static inline const char* getBuildApiPath(ApiType type)
{
	using SRC = ApiType;
	switch (type)
	{
	case SRC::DX11:		{ return "dx11"; }
				  //case SRC::DX12:		{ return ""; }
				  //case SRC::OpenGL:	{ return ""; }
	case SRC::Vulkan:	{ return "spirv"; }
					//case SRC::Metal:	{ return ""; }
	default:
		throw SGE_ERROR("unknow api type");
	}
}

}