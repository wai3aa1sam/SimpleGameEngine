#include <sge_core/string/FileGenerator.h>
#include "CompileRequest.h"

namespace sge {

#define ShaderCompilerType_ENUM_LIST(E) \
	E(None,) \
	E(SGE,) \
	E(GLSLC,) \
	E(SPIRV_CROSS,) \
//----
SGE_ENUM_CLASS(ShaderCompilerType, u8)

class ShaderGNUMake : public GNUMakeGenerator
{
	using Base = GNUMakeGenerator;
	//using CompileInfo = CompileInfo;
	struct Request : public RequestBase
	{
		const CompileInfo* compileInfo = nullptr;
	};
public:
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

		const char* nts_var(const char* name) { Base::var(tmp, name); return tmp.c_str(); }
	private:
		TempString tmp;
	};

	static Names s_names;

	ShaderGNUMake(const CompileInfo& cInfo)
		: Base(_request)
	{
		_request.compileInfo = &cInfo;
	}

	static void generate(const CompileInfo& cInfo_)
	{
		ShaderGNUMake make(cInfo_);

		auto& request  = make._request;
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;

		TempString tmp;
		FmtTo(tmp, "LocalTemp/Imported/{}/makeFile", cInfo.comileRequest.inputFilename);

		make.phony({"all", "clean"});

		make._init_includes(request);

		make._init_variables(request);

		make._init_build_target(request, ApiType::DX11);	make.nextLine();
		make._init_build_target(request, ApiType::Vulkan);	make.nextLine();

		make.flush(tmp);
	}

	static const char* getBuildApiPath(ApiType type)
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

private:
	void _init_variables(Request& request)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;
		
		TempString tmp;

		assignVariable(names.currentMakeFile,		":=", "$(lastword $(MAKEFILE_LIST))");
		//assignVariable(names.sgeRoot,				":=", "");		
		//assignVariable(names.projectRoot,			":=", "");	
		//assignVariable(names.shaderCompilerPath,	":=", cInfo.compilerPath);
		//assignVariable(names.shaderCompiler,		":=", cInfo.shaderCompiler);
		//assignVariable(names.glslc,					":=", "glslc");
		//assignVariable(names.shaderFilePath,		":=", cInfo.comileRequest.inputFilename);

		nextLine();
	}

	void _init_includes(Request& request)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;

		//includePath(cInfo.compilerMakePath);
		//includePath(cInfo.projectMakePath);

		nextLine();
	}

	struct BuildRequest
	{
		BuildRequest(TempString& tmp, const String& entry, ShaderStageMask mask, size_t passIndex, ApiType apiType)
			:
			_tmp(&tmp), _entry(&entry), _mask(mask), _passIndex(passIndex), _apiType(apiType)
		{
		}

		void reset(const String& entry, ShaderStageMask mask)
		{
			this->_entry	= &entry;
			this->_mask		= mask;
		}

		TempString*		_tmp;
		const String*	_entry;
		ShaderStageMask _mask;
		size_t			_passIndex;
		ApiType			_apiType;
	};

	void _init_build_target(Request& request, ApiType apiType)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;

		auto target = getBuildTargetName(apiType);

		auto ifeq = Ifeq::ctor(request, target, "1");
		nextLine();

		TempString tmp;
		size_t passIndex = 0;
		for (auto& pass : cInfo.shaderInfo.passes) {

			BuildRequest bReq(tmp, pass.vsFunc, ShaderStageMask::Vertex, passIndex, apiType);
			_init_bin(request, bReq); bReq.reset(pass.psFunc, ShaderStageMask::Pixel);
			_init_bin(request, bReq); 

			passIndex++;
		}
	}

	void _init_bin(Request& request, BuildRequest& buildReq)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;

		auto& tmp		= *buildReq._tmp;		(void) tmp;
		auto& entry		= *buildReq._entry;		(void) entry;
		auto& mask		=  buildReq._mask;		(void) mask;
		auto& passIndex	=  buildReq._passIndex;	(void) passIndex;
		auto& apiType	=  buildReq._apiType;	(void) apiType;

		const char* compilerName = getCompilerName(apiType);

		generateDepdency(request, buildReq);

		tmp.clear();

		const auto* apiName = getBuildBinName(apiType);

		if (entry.size()) {
			auto profile = getStageProfile(mask, apiType);
			auto binName = Fmt("$({})/pass{}/{}.bin", apiName, passIndex, profile);
			tmp.assign(binName);

			{ auto target0 = Target::ctor(request, "all", { binName });  }
			{ tmp += ".dep";  includePath(tmp); }
			{
				auto target1 = Target::ctor(request, binName);
				write(" "); write(names.nts_var(names.currentMakeFile));
				write(" "); write(names.nts_var(names.shaderFileRoot));
				if (compilerName) { write(" "); write(names.nts_var(compilerName)); }
				nextLine();
				_init_cli(request, buildReq);
				nextLine();
			}
		}
	}

	void _init_cli(Request& request, BuildRequest& buildReq)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;

		auto& tmp		= *buildReq._tmp;		(void) tmp;
		auto& entry		= *buildReq._entry;		(void) entry;
		auto& mask		=  buildReq._mask;		(void) mask;
		auto& passIndex	=  buildReq._passIndex;	(void) passIndex;
		auto& apiType	=  buildReq._apiType;	(void) apiType;

		using SRC = ApiType;
		switch (apiType)
		{
		case SRC::DX11:		
		{ 
			_init_cli_sgeShaderCompiler(request, buildReq);
		} break;

		case SRC::Vulkan:		
		{ 
			_init_cli_glslc(request, buildReq);
		} break;

		default:
			throw SGE_ERROR("unknow api type, getBuildBinName()");
		}

	}
	void _init_cli_sgeShaderCompiler(Request& request, BuildRequest& buildReq)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;
		auto& cReq	  = cInfo.comileRequest;

		onWrite(names.nts_var(names.shaderCompiler));

		const auto* profie	= getStageProfile(buildReq._mask, buildReq._apiType);
		auto& entry			= *buildReq._entry;

		{ auto bcmd = BeginCmd::ctor(request); write("-cwd=");			auto bstr = BeginString::ctor(request, names.nts_var(names.projectRoot)); }

		{ auto bcmd = BeginCmd::ctor(request); write("-x=");			write(cReq.language); }
		{ auto bcmd = BeginCmd::ctor(request); write("-profile=");		write(profie); }
		{ auto bcmd = BeginCmd::ctor(request); write("-entry=");		write(entry); }		
		{ auto bcmd = BeginCmd::ctor(request); write("-file=");			auto bstr = BeginString::ctor(request, names.nts_var(names.shaderFilePath)); }
		{ auto bcmd = BeginCmd::ctor(request); write("-out=");			auto bstr = BeginString::ctor(request, "$@"); }		
		{ 
			_init_cli_include(request, buildReq, "-I=");
		}

	}
	void _init_cli_glslc(Request& request, BuildRequest& buildReq)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;
		auto& cReq	  = cInfo.comileRequest;

		onWrite("cd "); write(names.nts_var(names.projectRoot)); write(" && ");
		write(names.nts_var(names.glslc));

		const auto* profie	= getGlslcStageProfile(buildReq._mask, buildReq._apiType);
		auto& entry			= *buildReq._entry;

		// -MD must be behide -fshader-stage and -fentry-point

		{ auto bcmd = BeginCmd::ctor(request); write("-x ");				write(cReq.language); }
		{ auto bcmd = BeginCmd::ctor(request); write("-fshader-stage=");	write(profie); }
		{ auto bcmd = BeginCmd::ctor(request); write("-fentry-point=");		write(entry); }	
		{ auto bcmd = BeginCmd::ctor(request); write("-MD ");				{ auto bstr = BeginString::ctor(request); write(names.nts_var(names.shaderFilePath)); } }
		{ auto bcmd = BeginCmd::ctor(request); write("-o ");				{ auto bstr = BeginString::ctor(request); write(names.nts_var(names.shaderBinRoot)); write("/"); write("$@"); } }
		{
			_init_cli_include(request, buildReq, "-I");
		}

	}
	void _init_cli_include(Request& request, BuildRequest& buildReq, StrView syntax)
	{
		auto& names	  = s_names;			  (void) names;
		{
			auto bcmd = BeginCmd::ctor(request); write(syntax);
			auto bstr = BeginString::ctor(request);
			//write("../../"); write(names.nts_var(names.builtInPath));
			write(names.nts_var(names.builtInRoot));
		}
		{
			auto bcmd = BeginCmd::ctor(request); write(syntax);
			auto bstr = BeginString::ctor(request);
			//write("../../"); write(names.nts_var(names.builtInShaderPath));
			write(names.nts_var(names.builtInShaderRoot));
		}
		/*for (auto& inc : cReq.includes)
		{
		auto bcmd = BeginCmd::ctor(request); write(syntax);
		auto bstr = BeginString::ctor(request);
		write(inc);
		}*/
	}

	static const char* getBuildTargetName(ApiType type)
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

	static const char* getBuildBinName(ApiType type)
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

	static const char* getCompilerName(ApiType type)
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

	static const char* getStageProfile(ShaderStageMask mask, ApiType type)
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

	static const char* getGlslcStageProfile(ShaderStageMask mask, ApiType type)
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

	static void generateDepdency(Request& request, BuildRequest& buildReq)
	{
		auto& cInfo		= *request.compileInfo; (void) cInfo;
		auto& names		= s_names;				(void) names;
		auto& cReq		= request.compileInfo->comileRequest; (void) cReq;

		auto& tmp		= *buildReq._tmp;		(void) tmp;
		auto& entry		= *buildReq._entry;		(void) entry;
		auto& mask		=  buildReq._mask;		(void) mask;
		auto& passIndex	=  buildReq._passIndex;	(void) passIndex;
		auto& apiType	=  buildReq._apiType;	(void) apiType;

		const auto* apiName = getBuildBinName(apiType);
		auto profile		= getStageProfile(mask, apiType);
		const char* apiPath = getBuildApiPath(apiType);

		tmp.clear();

		String content;
		String binName = Fmt("$({})/pass{}/{}.bin", apiName, passIndex, profile);

		content.reserve(4096);
		FmtTo(tmp, "LocalTemp/Imported/{}/{}/pass{}/{}.bin.dep", cInfo.comileRequest.inputFilename, apiPath, passIndex, profile);

		content += binName; content += ":\\\n";

		for (auto& inc : cReq.include.files)
		{
			content += "\t";
			content += inc;
			content += "\\\n";
		}

		File::writeFileIfChanged(tmp, content, false);
	}

#if 0
	void _init_dx11(Request& request)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;

		auto ifeq = Ifeq::ctor(request, names.buildDX11, "1");
		nextLine();

		TempString tmp;
		size_t passIndex = 0;
		for (auto& pass : cInfo.shaderInfo.passes) {

			init_dx11_bin(request, tmp, pass.vsFunc, ShaderStageMask::Vertex, passIndex);
			init_dx11_bin(request, tmp, pass.psFunc, ShaderStageMask::Pixel,  passIndex);

			passIndex++;
		}
	}
	void init_dx11_bin(Request& request, TempString& tmp, const String& func, ShaderStageMask mask, size_t passIndex)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;

		tmp.clear();

		if (func.size()) {
			auto profile = DX11Util::getDxStageProfile(mask);
			FmtTo(tmp, "$({})/pass{}/{}.bin", names.dx11, passIndex, profile);

			{ auto target0 = Target::ctor(request, "all", { tmp });  }

			{
				auto target1 = Target::ctor(request, tmp, { names.nts_var(names.currentMakeFile), } );
				onWrite(names.nts_var(names.shaderCompiler));
				nextLine();
			}
		}
	}

#endif // 0

protected:
	Request _request;
};

}