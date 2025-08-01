#pragma once

#include "Vertex.h"

namespace sge {


class VertexLayoutManager {
public:
	VertexLayoutManager();

	static VertexLayoutManager* instance();

	const VertexLayout* getLayout(VertexType type);

	VertexLayout* _createLayout(VertexType type);

	template<class VERTEX>
	void registerLayout() {
		VertexLayout* layout = _createLayout(VERTEX::kType);
		layout->stride = sizeof(VERTEX);
		layout->type = VERTEX::kType;
		VERTEX::onRegister(layout);
	}

private:
	Map<VertexType, VertexLayout>	_table;
};

using Vertex_Pos2f			= VertexT_Pos<Tuple2f>;

using Vertex_Pos			= VertexT_Pos<Tuple3f>;
using Vertex_PosNormal		= VertexT_Normal<Tuple3f, 1, Vertex_Pos>;

using Vertex_PosColor		= VertexT_Color<Color4b, 1, Vertex_Pos>;
using Vertex_PosColorNormal	= VertexT_Normal<Tuple3f, 1, Vertex_PosColor>;

template<u8 UV_COUNT> using Vertex_PosUv				= VertexT_Uv<Tuple2f, UV_COUNT, Vertex_Pos>;
template<u8 UV_COUNT> using Vertex_PosColorUv			= VertexT_Uv<Tuple2f, UV_COUNT, Vertex_PosColor>;

template<u8 UV_COUNT> using Vertex_PosNormalUv			= VertexT_Normal<Tuple3f,   1, Vertex_PosUv<UV_COUNT>>;
template<u8 UV_COUNT> using Vertex_PosColorNormalUv		= VertexT_Normal<Tuple3f,   1, Vertex_PosColorUv<UV_COUNT>>;

template<u8 UV_COUNT> using Vertex_PosTangentUv			= VertexT_Tangent<Tuple3f,  1, Vertex_PosNormalUv<UV_COUNT>>;
template<u8 UV_COUNT> using Vertex_PosColorTangentUv	= VertexT_Tangent<Tuple3f,  1, Vertex_PosColorNormalUv<UV_COUNT>>;

template<u8 UV_COUNT> using Vertex_PosBinormalUv		= VertexT_Binormal<Tuple3f, 1, Vertex_PosTangentUv<UV_COUNT>>;
template<u8 UV_COUNT> using Vertex_PosColorBinormalUv	= VertexT_Binormal<Tuple3f, 1, Vertex_PosColorTangentUv<UV_COUNT>>;

using Vertex_ImGui = VertexT_Color<Color4b, 1, VertexT_Uv<Tuple2f, 1, Vertex_Pos2f>>;

using Vertex_VertexId	= VertexT_VertexId<VertexBase>;

}