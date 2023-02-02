#pragma once

#include "Render_Common.h"
#include "RenderContext.h"
#include "shader/Material.h"
#include "textures/Texture.h"

namespace sge {

class RenderContext;
struct RenderContext_CreateDesc;

class RenderGpuBuffer;
struct RenderGpuBuffer_CreateDesc;

class Renderer : public NonCopyable {
public:
	static Renderer*	instance() { return s_instance; }

	enum class ApiType {
		None = 0,
		DX11,
		DX12,
		OpenGL,
		Vulkan,
		Metal,
	};

	struct CreateDesc {
		CreateDesc();
		ApiType		apiType;
		bool multithread : 1;
	};

	static Renderer* create(CreateDesc& desc);

	Renderer();
	virtual ~Renderer();

	const RenderAdapterInfo&	adapterInfo() const { return _adapterInfo; }

	bool vsync() const		{ return _vsync; }


	SPtr<RenderContext>		createContext	(RenderContext_CreateDesc&		desc);
	SPtr<RenderGpuBuffer>	createGpuBuffer	(RenderGpuBuffer_CreateDesc&	desc) { return onCreateGpuBuffer(desc); }
	SPtr<Texture2D>			createTexture2D	(Texture2D_CreateDesc&			desc) { return onCreateTexture2D(desc); }
	SPtr<Shader>			createShader	(StrView filename);
	SPtr<Shader>			createShader(Shader* shader, const ShaderPermutations& permuts);

	SPtr<Material>			createMaterial	()	{ return onCreateMaterial(); }

	void onShaderDestory(Shader* shader);

	struct StockTextures {
		SPtr<Texture2D>	white;
		SPtr<Texture2D>	black;
		SPtr<Texture2D>	red;
		SPtr<Texture2D>	green;
		SPtr<Texture2D>	blue;
		SPtr<Texture2D>	magenta;
		SPtr<Texture2D>	error;
	};

	StockTextures stockTextures;

	SPtr<Texture2D>	createSolidColorTexture2D(const Color4b& color);

	//SPtr<Shader> defaultShader() { static SPtr<Shader> p = createShader("Assets") }


private:

protected:
	virtual SPtr<RenderContext>		onCreateContext		(RenderContext_CreateDesc&		desc) = 0;
	virtual SPtr<RenderGpuBuffer>	onCreateGpuBuffer	(RenderGpuBuffer_CreateDesc&	desc) = 0;
	virtual SPtr<Texture2D>			onCreateTexture2D	(Texture2D_CreateDesc&			desc) = 0;
	virtual SPtr<Shader>			onCreateShader		(StrView filename) = 0;
	virtual SPtr<Shader>			onCreateShader		(Shader* shader, const ShaderPermutations& permuts) { return nullptr; };
	virtual SPtr<Material>			onCreateMaterial	() = 0;

	StringMap<Shader*>	_shaders;

	static Renderer*	s_instance;
	RenderAdapterInfo	_adapterInfo;
	bool _vsync : 1;

	class ShaderRefData
	{
		using Shaders	= Vector<SPtr<Shader>, 2>;
		using Materials = Vector<SPtr<Material>, 2>;
	public:
		Shader* findShader(StrView filename) 
		{
			auto it = getShaders(filename);
			if (it == nullptr)
				return nullptr;
			return it->front(); 
		}

		Shaders* getShaders(StrView filename)
		{
			auto it = _shaders.find(filename);
			if (it == _shaders.end())
				return nullptr;
			return &it->second; 
		}

		Materials* getMaterials(Shader* shader)
		{
			auto it = _materialTable.find(shader);
			if (it == _materialTable.end())
				return nullptr;
			return &it->second; 
		}

		SPtr<Shader> createShader(StrView filename_)
		{
			TempString tmpName = filename_;

			auto it = _shaders.find(tmpName.c_str());
			if (it != _shaders.end()) {
				return it->second.front();
			}

			auto& shaders = _shaders[tmpName.c_str()];
			SGE_ASSERT(shaders.size() == 0, "");

			auto* renderer = Renderer::instance();
			auto s = renderer->onCreateShader(tmpName);
			shaders.emplace_back(s.ptr());
			_materialTable[s.ptr()];
			return s;
		}

		SPtr<Shader> createShader(Shader* shader, const ShaderPermutations& permuts)
		{
			const auto& filename = shader->filename();

			auto it = _shaders.find(filename.c_str());
			if (it == _shaders.end()) {
				SGE_ASSERT(false, "no shader for permutation");
			}

			auto& shaders = _shaders[filename.c_str()];
			for (auto& s : shaders)
			{
				if (s->permutations() == permuts)
					return s;
			}

			auto* renderer = Renderer::instance();
			auto s = renderer->onCreateShader(shader, permuts);
			return shaders.emplace_back(s.ptr());
		}

		SPtr<Material> createMaterial()
		{
			auto* rdr = Renderer::instance();
			auto mtl = rdr->onCreateMaterial();
			return mtl;
		}

		void removeShader(Shader* shader)
		{
			if (shader->permutations().size() > 0)
				return;
			auto it = _materialTable.find(shader);
			if (it == _materialTable.end())
				return;

			//auto* renderer = Renderer::instance();
			auto& materials = it->second;
			for (auto& mtl : materials)
			{
				mtl->setShader(nullptr); // set default shader;
			}

			_shaders.erase(shader->filename().c_str());
			_materialTable.erase(shader);
		}

		void setMaterialShader(Material* mtl, Shader* shader)
		{
			auto& mtlTable = _materialTable[shader];
			for (auto& m : mtlTable)
			{
				if (m == mtl)
					return;
			}
			mtlTable.emplace_back(mtl);
		}

		const VectorMap<Material*, Shader*>& permutationRequest() const { return _permutationRequest; }

		void sendPermutationRequest(Material* mtl)
		{
			auto it = _permutationRequest.find(mtl);
			if (it != _permutationRequest.end())
				return;
			_permutationRequest[mtl];
		}

		void clearPermutationRequest()
		{
			_permutationRequest.clear();
		}

	private:
		Map<String, Shaders>			_shaders;
		Map<Shader*, Materials>			_materialTable;
		VectorMap<Material*, Shader*>	_permutationRequest; // only need Material*
	};

public:
	ShaderRefData& shaderRefData() { return _shaderRefData; }

private:
	ShaderRefData _shaderRefData;
	//ShaderCompile _shaderCompile;
};

}