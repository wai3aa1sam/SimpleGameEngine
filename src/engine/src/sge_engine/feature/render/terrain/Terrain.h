#pragma once
#include <sge_render/textures/Texture.h>
#include <sge_render/buffer/RenderGpuBuffer.h>
#include <sge_render/mesh/RenderMesh.h>

#include <sge_render/command/RenderRequest.h>

#include <sge_core/math/MathCamera3.h>

#include <sge_engine/components/Component.h>

namespace sge {
namespace my {

class Terrain;

struct Terrain_CreateDesc
{
	Vec2i patchCount{ 1, 1 };

	//Image heightMap;
	Image* heightMap = nullptr;
};

class _Terrain_Patch : public RefCountBase
{
public:
	//virtual ~_Terrain_Patch() = default;
	void create(Terrain* terrain, Shader* shader);

	Vec3f worldCenterPos() const;

	void setLOD(const Vec3f& viewPos);

	u8 tblr = 0b0000;
	int lod = 0;

	Vec2i id;
	Vec2f pos;
	Vec2f uv;

	SPtr<RenderGpuBuffer> spVertexBuffer;
	SPtr<RenderGpuBuffer> spIndexBuffer;
	SPtr<Material>		  _material;

private:
	Terrain* _terrain = nullptr;
};

class Terrain : public NonCopyable
{
	friend class _Terrain_Patch;
public:
	static constexpr int s_kMaxLOD = 10;
	static constexpr int s_kMaxCombination = 16;

	using CreateDesc = Terrain_CreateDesc;
	using Patch = _Terrain_Patch;
	using IndexChunks = Vector<Vector<SPtr<RenderGpuBuffer>, s_kMaxCombination>, Terrain::s_kMaxLOD>;
	using IndexType = u32;

	Terrain() = default;
	Terrain(CreateDesc& desc_);

	void create(CreateDesc& desc_);

	void render(RenderRequest& rdReq);

	Vec2i size() const;
	int patchIndex(Vec2i iPatch_) const;

	SPtr<Patch>& getPatch(Vec2i iPatch_);
	SPtr<Patch>& getPatch_unsafe(Vec2i iPatch_);
	SPtr<Patch>& getPatch_clamp(Vec2i iPatch_);

	bool checkBoundary(Vec2i iPatch_) const;
	void clampBoundary(Vec2i& iPatch_) const;

	Span<      SPtr<Patch>>	getPatches() { return _patches; }
	Span<const SPtr<Patch>>	getPatches() const { return _patches; }

	RenderMesh& getRenderMesh() { return _testRenderMesh; }
	const RenderMesh& getRenderMesh() const { return _testRenderMesh; }

	Vec2i getPatchCoord(const Vec3f& pos);
	Vec2f patchSize() const { return Vec2f::s_cast(_patchSize); }
	
	Vec3f terrainPos() const { return _terrainPos; }

public:
	Vec3f _terrainPos{ 0, 0, 0 };
	float _lodFactor = 5;
private:
	void _create(int width_, int height_) {};
	void _init();

	void update(RenderRequest& rdReq);

	void _error(StrView msg_);

private:

	Vec2i _size{ 0, 0 };
	Vec2i _patchCount;
	Vec2i _patchSize;
	int   _maxLodIndex = 0;

	Vector<SPtr<RenderGpuBuffer>, s_kMaxCombination> _indexBufferSPtrs;

	RenderMesh _testRenderMesh;

	Vector<SPtr<Patch>> _patches;

	IndexChunks _indexChunks;
	int _squareToTriangleRatio = 2;

	SPtr<Texture2D> _heightMap;

	SPtr<RenderGpuBuffer> spIndexBuffer;
};

#if 0
#pragma mark --- Terrain-Impl ---
#endif // 0
#if 1    // ImageInfo

SGE_INLINE Vec2i Terrain::size() const { return _size; }
SGE_INLINE int Terrain::patchIndex(Vec2i iPatch_) const { return _patchCount.x * iPatch_.y + iPatch_.x; }

SGE_INLINE SPtr<Terrain::Patch>& Terrain::getPatch(Vec2i iPatch_) { if (!checkBoundary(iPatch_)) _error("out of boundary"); return getPatch_unsafe(iPatch_); }
SGE_INLINE SPtr<Terrain::Patch>& Terrain::getPatch_unsafe(Vec2i iPatch_) { return _patches[patchIndex(iPatch_)]; }
SGE_INLINE SPtr<Terrain::Patch>& Terrain::getPatch_clamp(Vec2i iPatch_) { clampBoundary(iPatch_); return _patches[patchIndex(iPatch_)]; }

SGE_INLINE bool Terrain::checkBoundary(Vec2i iPatch_) const { return iPatch_.x >= 0 && iPatch_.x < _patchCount.x&& iPatch_.y >= 0 && iPatch_.y < _patchCount.y; }
SGE_INLINE void Terrain::clampBoundary(Vec2i& iPatch_) const { iPatch_.x = Math::clamp(iPatch_.x, 0, _patchCount.x - 1); iPatch_.y = Math::clamp(iPatch_.y, 0, _patchCount.y - 1); }

SGE_INLINE void Terrain::_error(StrView msg_) { throw SGE_ERROR("{}", msg_); }

#endif

#if 0
#pragma mark --- _Terrain_Patch-Impl ---
#endif // 0
#if 1    // ImageInfo

inline void _Terrain_Patch::create(Terrain * terrain, Shader * shader)
{
	_terrain = terrain;
	_material = Renderer::instance()->createMaterial();
	_material->setShader(shader);
}

inline Vec3f _Terrain_Patch::worldCenterPos() const 
{
	auto s   = _terrain->patchSize();
	auto centerPos = (Vec2f::s_cast(id) + 0.5f) * s;
	auto o = _terrain->terrainPos() + Vec3f(centerPos.x, 0, centerPos.y);
	return o;
}

inline void _Terrain_Patch::setLOD(const Vec3f& viewPos)
{
	auto distance = (worldCenterPos() - viewPos).length();
	auto d = _terrain->patchSize().x * _terrain->_lodFactor;
	lod = _terrain->_maxLodIndex - static_cast<int>(distance / d);
	lod = Math::clamp(lod, 0, _terrain->_maxLodIndex);
}

#endif

}

#if 1

class CTerrain : public Component {
	SGE_OBJECT_TYPE(CTerrain, Component)
public:
	CTerrain() {}

	my::Terrain _terrain;
};

template<> const TypeInfo* TypeOf<my::Terrain>();

#endif // 1


}
