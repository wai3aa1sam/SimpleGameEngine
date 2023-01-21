#pragma once
#include <sge_render/shader/ShaderInfo.h>

namespace sge {

struct CompileRequest
{
	String language;
	String inputFilename;
	String outputFilename;

	String profile;
	String entry;

	struct Include 
	{ 
		void reserve() { files.reserve(fileCount()); }
		void addFileCount()			{ _fileCount++; };
		int  fileCount()	const	{ return _fileCount; }

		Vector<String>	files;
		Vector<String, 4>	dirs;

	private:
		int _fileCount = 0;
	};
	struct Marco { String name; String value; };

	Include include;
	Vector<Marco, 12> marcos;
};

struct CompileInfo
{
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
		auto& files = include.files;
		auto& back = files.emplace_back();
		auto is_valid = s_resolve(back, include, shader_path, inc_path);
		if (!is_valid)
		{
			files.pop_back();
		}
		return is_valid;
	}
	
	template<class STRING>
	static bool s_resolve(STRING& out, const Include& include, StrView shader_path, StrView inc_path)
	{
		out.clear();

		auto& dirs  = include.dirs;

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

	const String& lastFile() const { return req().include.files.back(); }

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

}