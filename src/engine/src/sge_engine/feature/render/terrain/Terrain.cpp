#include <sge_engine-pch.h>
#include "Terrain.h"

#include "sge_render/Renderer.h"

#include "sge_render/vertex/VertexLayoutManager.h"

#define _SGE_TERRRAIN_DEBUG 0
#define _SGE_TERRRAIN_DEBUG_PRINT 0

namespace sge {

#if 1

template<> inline
const TypeInfo* TypeOf<my::Terrain>() 
{
	using This = my::Terrain;
	using TI_Base = TypeInfoInitNoBase<This>;

	class TI : public TI_Base {
	public:
		TI()
			: TI_Base("Terrain")
		{
			static FieldInfo fi[] = {

				{"_terrainPos",	&This::_terrainPos  },
				{"_lodFactor",	&This::_lodFactor   },
			};
			setFields(fi);
		}
	};
	static TI ti;
	return &ti;
}

const TypeInfo* CTerrain::s_getType() {
	class TI : public TI_Base {
	public:
		TI() {
			static FieldInfo fi[] = {
				{"_terrain",	&This::_terrain },
			};
			setFields(fi);
		}
	};
	static TI ti;
	return &ti;

}
#endif // 1

namespace my {

#if 0
#pragma mark --- LODIndices-Impl ---
#endif // 0
#if 1

struct LODIndices
{
	using IndexType = Terrain::IndexType;

	void create(int lod_, Vec2i vertexSize_, int squareToTriangleRatio_ = 4)
	{
		_init(lod_, vertexSize_, squareToTriangleRatio_);
		_createIndices();
	}

	void next()
	{
		_clear();
		currentLod--;
		if (currentLod < 0)
			return;

		_update();
		_createIndices();
	}

	void setSquareToTriangleRatio(int squareToTriangleRatio_)
	{
		squareToTriangleRatio = squareToTriangleRatio_;
	}

	u32 index(int x, int y, int width_) { return y * width_ + x; }
	u32 index(int x, int y) { return y * vertexSize.x + x; }

	u32 index(Vec2i coord_) { return coord_.y * vertexSize.x + coord_.x; }

	Vec2i indexToXY(u32 index_) { return { static_cast<int>(index_ % vertexSize.x), static_cast<int>(index_ / vertexSize.x) }; }
	u32 topToBottom(u32 index_) { auto coord = indexToXY(index_); return index(vertexSize - coord - 1); }

	Span<const u32> getTopEdgeIndices_lod0()		{ return Span<const u32>(lod0Indices.begin() + stride0 * 0, lod0Indices.begin() + stride0 * 1); }
	Span<const u32> getBottomEdgeIndices_lod0()		{ return Span<const u32>(lod0Indices.begin() + stride0 * 1, lod0Indices.begin() + stride0 * 2); }
	Span<const u32> getLeftEdgeIndices_lod0()		{ return Span<const u32>(lod0Indices.begin() + stride0 * 2, lod0Indices.begin() + stride0 * 3); }
	Span<const u32> getRightEdgeIndices_lod0()		{ return Span<const u32>(lod0Indices.begin() + stride0 * 3, lod0Indices.begin() + stride0 * 4); }

	Span<const u32> getTopEdgeIndices_lod1()		{ return Span<const u32>(lod1Indices.begin() + stride1 * 0, lod1Indices.begin() + stride1 * 1); }
	Span<const u32> getBottomEdgeIndices_lod1()		{ return Span<const u32>(lod1Indices.begin() + stride1 * 1, lod1Indices.begin() + stride1 * 2); }
	Span<const u32> getLeftEdgeIndices_lod1()		{ return Span<const u32>(lod1Indices.begin() + stride1 * 2, lod1Indices.begin() + stride1 * 3); }
	Span<const u32> getRightEdgeIndices_lod1()		{ return Span<const u32>(lod1Indices.begin() + stride1 * 3, lod1Indices.begin() + stride1 * 4); }

	Span<const u32> getInsideIndices() { return Span<const u32>(tmpIndices.begin(), tmpIndices.end()); }

private:
	void _init(int lod_, Vec2i vertexSize_, int squareToTriangleRatio_)
	{
		SGE_ASSERT(!_isCreated);
		SGE_ASSERT(vertexSize_.x == vertexSize_.y);
		_isCreated = true;

		vertexSize = vertexSize_;

		int lodIndex = Math::log2(vertexSize.x - 1) - 1;
		currentLod = lodIndex;
		maxLod = lodIndex;

		squareToTriangleRatio = squareToTriangleRatio_;

		_update();

		lod0Indices.reserve(stride0 * 4);
		lod1Indices.reserve(stride1 * 4);

		tmpIndices.reserve(maxLod * squareToTriangleRatio * 3);
	}

	void _emplace_back(Vector<u32>& dst_, int x0_, int y0_, int x1_, int y1_, int x2_, int y2_)
	{
		dst_.emplace_back(index(x0_, y0_)); dst_.emplace_back(index(x2_, y2_)); dst_.emplace_back(index(x1_, y1_));
	}
	void _emplace_back_TopBottom(Vector<u32>& dst_, int x0_, int y0_, int x1_, int y1_, int x2_, int y2_)
	{
		dst_.emplace_back(index(x0_, y0_));	dst_.emplace_back(index(x2_, y2_)); dst_.emplace_back(index(x1_, y1_));
		tmpIndices.emplace_back(topToBotton_Index(x0_, y0_)); tmpIndices.emplace_back(topToBotton_Index(x1_, y1_)); tmpIndices.emplace_back(topToBotton_Index(x2_, y2_)); 
	}
	void _emplace_back_LeftRight(Vector<u32>& dst_, int x0_, int y0_, int x1_, int y1_, int x2_, int y2_)
	{
		dst_.emplace_back(index(x0_, y0_)); dst_.emplace_back(index(x2_, y2_)); dst_.emplace_back(index(x1_, y1_)); 
		tmpIndices.emplace_back(leftToRight_Index(x0_, y0_)); tmpIndices.emplace_back(leftToRight_Index(x1_, y1_)); tmpIndices.emplace_back(leftToRight_Index(x2_, y2_));
	}

	void _createIndices()
	{
		int end = static_cast<int>(Math::pow(2.0f, static_cast<float>(currentLod))) - 1;

		auto factor0 = Math::pow2(inverseLod);		// 1, 2, 4, ...
		auto factor1 = Math::pow2(inverseLod + 1);	// 2, 4, 8, ...
		auto factor2 = Math::pow2(inverseLod + 2);	// 4, 8, 16, ...

		_createTopBottonEdgeIndices(end, factor0, factor1, factor2);
		_createLeftRightEdgeIndices(end, factor0, factor1, factor2);
		_createInsideIndices(end, factor0, factor1, factor2);
	}

	void _mergeAndClearIndicesChunk(Vector<u32>& dst_, size_t offset_, Vector<u32>& src_)
	{
		SGE_ASSERT(dst_.capacity() - dst_.size() >= src_.size());
		if (src_.size() == 0)
			return;

		dst_.resize(dst_.size() + src_.size());
		auto* pDst = dst_.data() + offset_;
		memcpy(pDst, src_.data(), src_.size() * sizeof(u32));
		src_.clear();
	}

	void _mergeIndicesChunk(Vector<u32>& dst_, size_t offset_, Vector<u32>& src_)
	{
		if (src_.size() == 0)
			return;

		dst_.resize(dst_.size() + src_.size());
		auto* pDst = dst_.data() + offset_;
		memcpy(pDst, src_.data(), src_.size() * sizeof(u32));
	}

	void _createTopBottonEdgeIndices(int end_, int factor0_, int factor1_, int factor2_)
	{
		auto end = end_, factor0 = factor0_, factor1 = factor1_, factor2 = factor2_;

		// LOD: lod - 1
		{
			if (end == 0)
			{
				auto start = 0 * factor1;
				_emplace_back_TopBottom(lod0Indices, start + 0, 0, start + factor1, 0, start + factor0, factor0);
			}

			// \/\ \/\ \/\ \/	eg. for lod == 2 (x vertex == 9)
			for (int i = 0; i < end; i++)
			{
				auto start = i * factor1;
				_emplace_back_TopBottom(lod0Indices, start + 0, 0, start + factor1, 0, start + factor0, factor0);
				_emplace_back_TopBottom(lod0Indices, start + factor1, 0, start + factor0 * 3, factor0, start + factor0, factor0);

				if (i == end - 1)
					_emplace_back_TopBottom(lod0Indices, start + factor1, 0, start + factor2, 0, start + 3 * factor0, factor0);
			}
			_mergeAndClearIndicesChunk(lod0Indices, lod0Indices.size(), tmpIndices);
		}

		// LOD: lod
		{
			if (end == 0)
			{
				auto start = 0 * factor1;
				_emplace_back_TopBottom(lod1Indices, start + 0, 0, start + factor0, 0, start + factor0, factor0);	// \|
				_emplace_back_TopBottom(lod1Indices, start + factor0, 0, start + factor1, 0, start + factor0, factor0);	//   |/
			}

			// \|/\ \|/|\ \|/\ \|/	eg. for lod == 2 (x vertex == 9)
			for (int i = 0; i < end; i++)
			{
				auto start = i * factor1;
				_emplace_back_TopBottom(lod1Indices, start + 0, 0, start + factor0, 0, start + factor0, factor0);	// \|
				_emplace_back_TopBottom(lod1Indices, start + factor0, 0, start + factor1, 0, start + factor0, factor0);	//   |/
				_emplace_back_TopBottom(lod1Indices, start + factor1, 0, start + factor0 * 3, factor0, start + factor0, factor0);	//    /\

				if (i == end - 1)
				{
					_emplace_back_TopBottom(lod1Indices, start + factor1, 0, start + factor0 * 3, 0, start + factor0 * 3, factor0);
					_emplace_back_TopBottom(lod1Indices, start + 3 * factor0, 0, start + factor2, 0, start + factor0 * 3, factor0);
				}
			}
			// merge bottom edge to the main indices
			_mergeAndClearIndicesChunk(lod1Indices, lod1Indices.size(), tmpIndices);
		}
	}
	void _createLeftRightEdgeIndices(int end_, int factor0_, int factor1_, int factor2_)
	{
		auto end = end_, factor0 = factor0_, factor1 = factor1_, factor2 = factor2_;

		// LOD: lod - 1
		{
			if (end == 0)
			{
				auto start = 0 * factor1;
				_emplace_back_LeftRight(lod0Indices, 0, start + 0, factor0, start + factor0, 0, start + factor1);
			}

			// similar to top
			for (int i = 0; i < end; i++)
			{
				auto start = i * factor1;
				_emplace_back_LeftRight(lod0Indices, 0, start + 0, factor0, start + factor0, 0, start + factor1);
				_emplace_back_LeftRight(lod0Indices, factor0, start + factor0, factor0, start + factor0 * 3, 0, start + factor1);

				if (i == end - 1)
					_emplace_back_LeftRight(lod0Indices, 0, start + factor1, factor0, start + factor0 * 3, 0, start + factor2);
			}
			_mergeAndClearIndicesChunk(lod0Indices, lod0Indices.size(), tmpIndices);
		}

		// LOD: lod
		{
			if (end == 0)
			{
				auto start = factor1 * 0;
				_emplace_back_LeftRight(lod1Indices, 0, start + 0, factor0, start + factor0, 0, start + factor1);
				_emplace_back_LeftRight(lod1Indices, 0, start + factor0, factor0, start + factor0, 0, start + factor1);
			}
			for (int i = 0; i < end; i++)
			{
				auto start = i * factor1;
				_emplace_back_LeftRight(lod1Indices, 0, start + 0, factor0, start + factor0, 0, start + factor0);
				_emplace_back_LeftRight(lod1Indices, 0, start + factor0, factor0, start + factor0, 0, start + factor1);
				_emplace_back_LeftRight(lod1Indices, factor0, start + factor0, factor0, start + factor0 * 3, 0, start + factor1);

				//auto start = i * 2;
				//_emplace_back_LeftRight(lod1Indices, 0, start + 0, 1, start + 1, 0, start + 1);
				//_emplace_back_LeftRight(lod1Indices, 0, start + 1, 1, start + 1, 0, start + 2);
				//_emplace_back_LeftRight(lod1Indices, 1, start + 1, 1, start + 3, 0, start + 2);


				if (i == end - 1)
				{
					_emplace_back_LeftRight(lod1Indices, 0, start + factor1, factor0, start + factor0 * 3, 0, start + factor0 * 3);
					_emplace_back_LeftRight(lod1Indices, 0, start + factor0 * 3, factor0, start + factor0 * 3, 0, start + factor2);

					//_emplace_back_LeftRight(lod1Indices, 0, start + 2, 1, start + 3, 0, start + 3);
					//_emplace_back_LeftRight(lod1Indices, 0, start + 3, 1, start + 3, 0, start + 4);
				}
			}
			// merge right edge to the main indices
			_mergeAndClearIndicesChunk(lod1Indices, lod1Indices.size(), tmpIndices);
		}
	}
	void _createInsideIndices(int end_, int factor0_, int factor1_, int factor2_)
	{
		auto end = end_, factor0 = factor0_, factor1 = factor1_;
		// split inside square to triangle
		tmpIndices.clear();
		if (squareToTriangleRatio == 4)
		{
			for (int j = 0; j < end_; j++)
			{
				auto yStart = j * factor1 + factor0;
				for (int i = 0; i < end; i++)
				{
					auto xStart = i * factor1 + factor0;

					_emplace_back(tmpIndices, xStart, yStart, xStart + factor1, yStart, xStart + factor0, yStart + factor0);
					_emplace_back(tmpIndices, xStart + factor1, yStart, xStart + factor1, yStart + factor1, xStart + factor0, yStart + factor0);
					_emplace_back(tmpIndices, xStart + factor0, yStart + factor0, xStart + factor1, yStart + factor1, xStart, yStart + factor1);
					_emplace_back(tmpIndices, xStart, yStart, xStart + factor0, yStart + factor0, xStart, yStart + factor1);
				}
			}
		}
		else if (squareToTriangleRatio == 2)
		{
			for (int j = 0; j < end_; j++)
			{
				auto yStart = j * factor1 + factor0;
				for (int i = 0; i < end; i++)
				{
					auto xStart = i * factor1 + factor0;

					_emplace_back(tmpIndices, xStart, yStart, xStart + factor1, yStart, xStart + factor1, yStart + factor1);
					_emplace_back(tmpIndices, xStart, yStart, xStart + factor1, yStart + factor1, xStart, yStart + factor1);
				}
			}
		}
		else
		{
			throw SGE_ERROR("invalid squareToTriangleRatio");
		}
	}

	u32 topToBotton_Index(int x_, int y_) { return index(x_, vertexSize.y - 1 - y_); }
	u32 leftToRight_Index(int x_, int y_) { return index(vertexSize.x - 1 - x_, y_); }

	void _clear()
	{
		lod0Indices.clear();
		lod1Indices.clear();
		tmpIndices.clear();
		//stride0 = 0, stride1 = 0;
	}
	void _update()
	{
		inverseLod = maxLod - currentLod;
		auto halfWidth = Math::pow2(currentLod + 1) / 2;
		stride0 = (halfWidth * 1 + halfWidth - 1) * 3;
		stride1 = (halfWidth * 2 + halfWidth - 1) * 3;
	}

public:
	bool _isCreated = false;
	Vec2i vertexSize;
	Vector<u32> lod0Indices;
	Vector<u32> lod1Indices;
	Vector<u32> tmpIndices;

	int currentLod = 0;
	int maxLod = 0;
	int inverseLod = 0;
	u32 stride0 = 0, stride1 = 0;

	int squareToTriangleRatio = 0;
};

class ChunkIndexGenerator
{
public:
	using IndexChunks = Terrain::IndexChunks;
	using IndexType = Terrain::IndexType;

	void generate(IndexChunks& indexChunks_, Vec2i vertexSize_)
	{
		pOut = &indexChunks_;

		int lod = Math::log2(vertexSize_.x - 1);
		lod = lod <= 0 ? 1 : lod;
		pOut->resize(lod);
		_maxLod = lod - 1;

		LODIndices lodIndices;
		lodIndices.create(_maxLod, vertexSize_);

#if 1
		auto* renderer = Renderer::instance();
		for (int j = _maxLod; j >= 0; j--)
		{
			int tblr = 0b00000000;
			auto& indicesChunks = (*pOut)[j];
			indicesChunks.resize(Terrain::s_kMaxCombination);
			for (size_t i = 0; i < Terrain::s_kMaxCombination; i++)
			{
				// Top
				if (BitUtil::hasBit(tblr, 3))		_copyTo(indices, lodIndices.getTopEdgeIndices_lod1());
				else								_copyTo(indices, lodIndices.getTopEdgeIndices_lod0());

				// Bottom
				if (BitUtil::hasBit(tblr, 2))		_copyTo(indices, lodIndices.getBottomEdgeIndices_lod1());
				else								_copyTo(indices, lodIndices.getBottomEdgeIndices_lod0());

				// Left
				if (BitUtil::hasBit(tblr, 1))		_copyTo(indices, lodIndices.getLeftEdgeIndices_lod1());
				else								_copyTo(indices, lodIndices.getLeftEdgeIndices_lod0());

				// Right
				if (BitUtil::hasBit(tblr, 0))		_copyTo(indices, lodIndices.getRightEdgeIndices_lod1());
				else								_copyTo(indices, lodIndices.getRightEdgeIndices_lod0());

				_copyTo(indices, lodIndices.getInsideIndices());

				auto& dst = indicesChunks[tblr];

				RenderGpuBuffer::CreateDesc desc;
				desc.type = RenderGpuBufferType::Index;
				ByteSpan indexData = spanCast<const u8, const u32>(indices);
				desc.bufferSize = indexData.size();
				desc.stride = RenderDataTypeUtil::getByteSize(RenderDataTypeUtil::get<IndexType>()) * 8;

				auto sp = renderer->createGpuBuffer(desc);
				dst = std::move(sp);
				dst->uploadToGpu(indexData);

				indices.clear();
				tblr++;
			}
			lodIndices.setSquareToTriangleRatio(2);
			lodIndices.next();
		}

#endif // 0

	}

private:
	void _copyTo(Vector<u32>& dst_, Span<const u32> src_)
	{
		auto oldSize = dst_.size();
		dst_.resize(dst_.size() + src_.size());
		memcpy(dst_.data() + oldSize, src_.data(), src_.size() * sizeof(Terrain::IndexType));
	}

private:
	IndexChunks* pOut = nullptr;
	int _maxLod = 0;

	Vector<u32> indices;
};

#endif // 1

#if 0
#pragma mark --- Terrain-Impl ---
#endif // 0
#if 1

Terrain::Terrain(CreateDesc& desc_)
{
	_size = desc_.heightMap->size();
}

void Terrain::create(CreateDesc& desc_)
{
	auto* renderer = Renderer::instance();

	auto mapSize = desc_.heightMap->size();
	_size = mapSize;

	_patchCount = desc_.patchCount;
	_patchSize = _size / _patchCount;
	_patches.reserve(_patchCount.x * _patchCount.y);

	int lodIndex = Math::log2(_patchSize.x) - 1;
	if (lodIndex >= s_kMaxLOD)
		throw SGE_ERROR("lodIndex > kMaxLOD");

	_maxLodIndex = lodIndex;

	_patchSize.x = Math::pow2(_maxLodIndex + 1) + 1;
	_patchSize.y = _patchSize.x;
	_size = _patchCount * _patchSize;

	ChunkIndexGenerator generator;
	if (_indexChunks.size() == 0)
		generator.generate(_indexChunks, _patchSize);

	_init();

	{
		auto shader = Renderer::instance()->createShader("Assets/Shaders/my_terrain.shader");
		for (auto& patch : getPatches())
		{
			patch->create(this, shader);
		}
	}
	{
		Texture2D_CreateDesc tex_desc;
		tex_desc.size = desc_.heightMap->size();
		tex_desc.colorType = desc_.heightMap->colorType();
		tex_desc.imageToUpload.copy(*desc_.heightMap);
		tex_desc.samplerState.filter = TextureFilter::Point;
		tex_desc.samplerState.wrapU = TextureWrap::Clamp;
		tex_desc.samplerState.wrapV = TextureWrap::Clamp;

		_heightMap = renderer->createTexture2D(tex_desc);
	}
}

void Terrain::render(RenderRequest& rdReq)
{
	update(rdReq);

	auto& renderMesh = getRenderMesh();
	auto& subMesh = renderMesh.subMeshes()[0];

	for (auto& patch : getPatches())
	{
		auto* cmd = rdReq.addDrawCall();

		auto& mtl = patch->_material;

		if (!mtl) { SGE_ASSERT(false); return; }

		rdReq.setMaterialCommonParams(mtl, Mat4f::s_identity());

		mtl->setParam("heightTex",		_heightMap);
		mtl->setParam("_patch_id",		patch->id);
		mtl->setParam("_patch_pos",		patch->pos);
		mtl->setParam("_patch_uv",		patch->uv);
		mtl->setParam("_terrainPos",	_terrainPos);


#if _DEBUG
		cmd->debugLoc = SGE_LOC;
#endif

		cmd->material			= mtl;
		cmd->materialPassIndex	= 0;
		cmd->primitive			= RenderPrimitiveType::Triangles;
		cmd->vertexLayout		= subMesh.vertexLayout();
		cmd->vertexBuffer		= patch->spVertexBuffer;
		cmd->vertexCount		= patch->spVertexBuffer->elementCount();
		cmd->vertexOffset		= 0;

		cmd->indexBuffer		= patch->spIndexBuffer;
		cmd->indexType			= RenderDataTypeUtil::get<Terrain::IndexType>();
		cmd->indexCount			= patch->spIndexBuffer->elementCount();
		cmd->indexOffset		= 0;
	}
}

void Terrain::update(RenderRequest& rdReq)
{

	for (int py = 0; py < _patchCount.y; py++)
	{
		for (int px = 0; px < _patchCount.x; px++)
		{
			auto& patch = getPatch_unsafe(Vec2i(px, py));
			patch->setLOD(rdReq.cameraPos);
		}
	}

	/*
	--------> +x
	|
	|
	\|/
	+y
	*/

	Vec2f invPatchCount{ 1.0f / _patchCount.x, 1.0f / _patchCount.y };
	for (int py = 0; py < _patchCount.y; py++)
	{
		for (int px = 0; px < _patchCount.x; px++)
		{
			auto& current	= getPatch_unsafe(Vec2i{ px,	 py } );
			auto& top		= getPatch_clamp( Vec2i{ px,	 py - 1 });
			auto& bottom	= getPatch_clamp( Vec2i{ px,	 py + 1 });
			auto& left		= getPatch_clamp( Vec2i{ px - 1, py });
			auto& right		= getPatch_clamp( Vec2i{ px + 1, py });

			u8 tblr = 0b0000;
			if (top->lod	> current->lod)	BitUtil::set(tblr,	3);
			if (bottom->lod > current->lod)	BitUtil::set(tblr,	2);
			if (left->lod	> current->lod)	BitUtil::set(tblr,	1);
			if (right->lod	> current->lod)	BitUtil::set(tblr,	0);

			current->spIndexBuffer.reset(_indexChunks[current->lod][tblr]);
			current->uv = { invPatchCount.x * px, invPatchCount.y * py };
		}
	}
}

void Terrain::_init()
{
	auto vertexCount = _patchSize.x * _patchSize.y;

	EditMesh tmpEditMesh;
	EditMesh& editMesh = tmpEditMesh;
	editMesh.pos.reserve(vertexCount);
	editMesh.uv[0].reserve(vertexCount);					// TODO: remove
	editMesh.color.resize(vertexCount);						// TODO: remove
	editMesh.normal.resize(vertexCount);					// TODO: remove

	{
		Vec2i iPatch_{ 0, 0 };
		Vec2i offset{ iPatch_.x * _patchSize.x, iPatch_.y * _patchSize.y };
		//Vec2f posOffset		{	 -_patchSize.x / 2 * offset.x, -_patchSize.y / 2 * offset.y};
		Vec2f posOffset{ 0.0f, 0.0f };

		Vec2f uvFactor{ 1.0f / (_patchCount.x * (_patchSize.x - 1)), 1.0f / (_patchCount.y * (_patchSize.y - 1)) };
		for (int py = 0; py < _patchSize.y; py++)
		{
			for (int px = 0; px < _patchSize.x; px++)
			{
				auto height = 0.0f;
				editMesh.pos.emplace_back(posOffset.x + px, height, posOffset.y + py);
				editMesh.uv[0].emplace_back(px * uvFactor.x, py * uvFactor.y);
			}
		}
	}

	auto& renderMesh = _testRenderMesh;
	renderMesh.create(editMesh);

	auto& subMesh = renderMesh.subMeshes()[0];

	u8 tblr = 0b00000000;

	for (int py = 0; py < _patchCount.y; py++)
	{
		for (int px = 0; px < _patchCount.x; px++)
		{
			Vec2i offset{ px * (_patchSize.x - 1), py * (_patchSize.y - 1) };
			auto& patch = _patches.emplace_back(new Patch);

			patch->spVertexBuffer = subMesh.vertexBuffer();
			patch->spIndexBuffer.reset(_indexChunks[_maxLodIndex][tblr + 0]);

			patch->id  = Vec2i{ px, py };
			patch->pos = Vec2f::s_cast(offset);
		}
	}
}

Vec2i Terrain::getPatchCoord(const Vec3f& camPos)
{
	Vec2i patch{ 0, 0 };
	auto pos = camPos - _terrainPos;
	patch.x = static_cast<int>(pos.x / _patchSize.x);
	patch.y = static_cast<int>(pos.z / _patchSize.y);
	clampBoundary(patch);
	return patch;
}

#endif // 1


}
}