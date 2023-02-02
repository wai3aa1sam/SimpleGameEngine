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

	bool _isGeneratedSgeShaderCompilerMake = false;

	ShaderGNUMake(const CompileInfo& cInfo)
		: Base(_request)
	{
		_request.compileInfo = &cInfo;
	}

	TempString nts_var(const char* name) { TempString tmp; Base::var(tmp, name); return tmp.c_str(); }

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

		if (apiType == ApiType::DX11 && _isGeneratedSgeShaderCompilerMake)
			return;

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
				write(" "); write(nts_var(names.currentMakeFile));
				write(" "); write(nts_var(names.shaderFileRoot));
				if (compilerName) { write(" "); write(nts_var(compilerName)); }
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

		_isGeneratedSgeShaderCompilerMake = true;

		onWrite(nts_var(names.shaderCompiler));

		{ auto bcmd = BeginCmd::ctor(request); write("-cwd=");			auto bstr = BeginString::ctor(request, nts_var(names.projectRoot)); }

		{ auto bcmd = BeginCmd::ctor(request); write("-makeCompile"); }
		{ auto bcmd = BeginCmd::ctor(request); write("-generateMake"); }

		#if 0
		auto& cReq	  = cInfo.comileRequest;  (void) cReq;
		const auto* profie	= getStageProfile(buildReq._mask, buildReq._apiType);
		auto& entry			= *buildReq._entry;

		{ auto bcmd = BeginCmd::ctor(request); write("-x=");			write(cReq.language); }
		{ auto bcmd = BeginCmd::ctor(request); write("-profile=");		write(profie); }
		{ auto bcmd = BeginCmd::ctor(request); write("-entry=");		write(entry); }		
		{ auto bcmd = BeginCmd::ctor(request); write("-file=");			auto bstr = BeginString::ctor(request, nts_var(names.shaderFilePath)); }
		{ auto bcmd = BeginCmd::ctor(request); write("-out=");			auto bstr = BeginString::ctor(request, "$@"); }		
		{ 
			_init_cli_include(request, buildReq, "-I=");
			_init_cli_marco(request, buildReq, "-D");
		}
		#else

		{ auto bcmd = BeginCmd::ctor(request); write("-file=");			auto bstr = BeginString::ctor(request, nts_var(names.shaderFilePath)); }

		#endif // 0

	}
	void _init_cli_glslc(Request& request, BuildRequest& buildReq)
	{
		auto& cInfo	  = *request.compileInfo; (void) cInfo;
		auto& names	  = s_names;			  (void) names;
		auto& cReq	  = cInfo.comileRequest;

		onWrite("cd "); write(nts_var(names.projectRoot)); write(" && ");
		write(nts_var(names.glslc));

		const char* profie	= getGlslcStageProfile(buildReq._mask);
		auto& entry			= *buildReq._entry;

		// -MD must be behide -fshader-stage and -fentry-point

		{ auto bcmd = BeginCmd::ctor(request); write("-x ");				write(cReq.language); }
		{ auto bcmd = BeginCmd::ctor(request); write("-fshader-stage=");	write(profie); }
		{ auto bcmd = BeginCmd::ctor(request); write("-fentry-point=");		write(entry); }	
		{ auto bcmd = BeginCmd::ctor(request); write("-MD ");				{ auto bstr = BeginString::ctor(request); write(nts_var(names.shaderFilePath)); } }
		{ auto bcmd = BeginCmd::ctor(request); write("-o ");				{ auto bstr = BeginString::ctor(request); write(nts_var(names.shaderBinRoot)); write("/"); write("$@"); } }
		{
			_init_cli_include(request, buildReq, "-I");
			_init_cli_marco(request, buildReq, "-D");
		}

	}
	void _init_cli_include(Request& request, BuildRequest& buildReq, StrView syntax)
	{
		auto& cInfo	  = *request.compileInfo;
		auto& cReq	  = cInfo.comileRequest;

		for (auto& inc : cReq.include.dirs())
		{
			auto bcmd = BeginCmd::ctor(request); write(syntax);
			auto bstr = BeginString::ctor(request);
			write(inc);
		}
	}
	void _init_cli_marco(Request& request, BuildRequest& buildReq, StrView syntax)
	{
		auto& cInfo	  = *request.compileInfo;
		auto& cReq	  = cInfo.comileRequest;

		for (auto& mar : cReq.marcos)
		{
			auto bcmd = BeginCmd::ctor(request); write(syntax);
			auto bstr = BeginString::ctor(request);
			write(mar.name); write(" = "); write(mar.value); 
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

		for (auto& inc : cReq.include.files())
		{
			content += "\t";
			content += inc;
			content += "\\\n";
		}

		File::writeFileIfChanged(tmp, content, false);
	}
protected:
	Request _request;
};

}