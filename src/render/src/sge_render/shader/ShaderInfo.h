#pragma once

#include <sge_render/RenderDataType.h>
#include <sge_render/vertex/Vertex.h>
#include <sge_core/serializer/json/JsonUtil.h>
#include "RenderState.h"

namespace sge {

enum class ShaderStageMask {
	None,
	Vertex	= 1 << 0,
	Pixel	= 1 << 1,
};

inline bool parseShaderStageMask(ShaderStageMask& outValue, StrView str) {

	if (str == "vs_5_0") { outValue = ShaderStageMask::Vertex;	return true; }
	if (str == "ps_5_0") { outValue = ShaderStageMask::Pixel;	return true; }
	return false;
}

#define ShaderPropType_ENUM_LIST(E) \
	E(None,) \
	E(Int,) \
	E(Float,) \
	E(Vec2f,) \
	E(Vec3f,) \
	E(Vec4f,) \
	E(Color4f,) \
	E(Texture2D,) \
//----
SGE_ENUM_CLASS(ShaderPropType, u8)

struct ShaderPropTypeUtil {	
	ShaderPropTypeUtil() = delete;

	using Type = ShaderPropType;

	template<class T> static constexpr Type get();

	template<> static constexpr	Type get<i32>()		{ return Type::Int; }
	template<> static constexpr	Type get<f32>()		{ return Type::Float; }
	template<> static constexpr	Type get<Vec2f>()	{ return Type::Vec2f; }
//	template<> static constexpr	Type get<Vec3f>()	{ return Type::Vec3f; }
//	template<> static constexpr	Type get<Vec4f>()	{ return Type::Vec4f; }
	template<> static constexpr	Type get<Color4f>()	{ return Type::Color4f; }
};

struct ShaderInfo {
	struct Prop {
		ShaderPropType	propType = ShaderPropType::None;
		String		name;
		String		displayName;
		String		defaultValue;

		template<class SE>
		void onJson(SE & se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, displayName);
			SGE_NAMED_IO(se, defaultValue);
		}
	};

	struct Pass {
		String name;
		String vsFunc;
		String psFunc;
		RenderState	renderState;

		template<class SE>
		void onJson(SE & se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, vsFunc);
			SGE_NAMED_IO(se, psFunc);
			SGE_NAMED_IO(se, renderState);
		}
	};

	struct Permutation
	{
		String name;
		//String value;
		Vector<String, 2> values;

		bool isValueExist(StrView val) const
		{
			return findValueIdx(val) != -1;
		}

		int findValueIdx(StrView val) const
		{
			for (int i = 0; i < values.size(); ++i)
			{
				if (val.compare(values[i]) == 0)
					return i;
			}
			return -1;
		}

		template<class SE>
		void onJson(SE& se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, values);
		}
	};

	bool isPermutationValid(StrView name, StrView value) const
	{
		for (auto& permut : permuts)
		{
			if (!(name == permut.name))
				continue;
			for (auto& v : permut.values)
			{
				if (!(value == v))
					continue;
				return true;
			}
		}
		return false;
	}
	int findPermutationNameIdx(StrView name) const
	{
		for (int i = 0; i < permuts.size(); i++)
		{
			auto& permut = permuts[i];
			if (name == permut.name)
				return i;
		}
		return -1;
	}

	using Permutations = Vector<Permutation, 2>;

	Vector<Prop, 8>		props;
	Vector<Pass, 1>		passes;
	Permutations		permuts;

	void clear();

	template<class SE>
	void onJson(SE & se) {
		SGE_NAMED_IO(se, props);
		SGE_NAMED_IO(se, passes);
		SGE_NAMED_IO(se, permuts);
	}
};

class ShaderStageInfo {
public:
	using DataType		= RenderDataType;

	void loadFile(StrView filename_) {
		filename = filename_;
		JsonUtil::readFile(filename_, *this);
	}

	String	filename;
	String	profile;
	ShaderStageMask	stage = ShaderStageMask::None;

	class Param {
	public:
		String name;
		DataType dataType;
		i16	bindPoint = 0;
		i16	bindCount = 0;

		template<class SE>
		void onJson(SE & se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, dataType);
			SGE_NAMED_IO(se, bindPoint);
			SGE_NAMED_IO(se, bindCount);
		}
	};

	class Input {
	public:
		String			name;
		VertexSemantic	semantic = VertexSemantic::None;
		RenderDataType	dataType = RenderDataType::None;

		template<class SE>
		void onJson(SE & se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, semantic);
			SGE_NAMED_IO(se, dataType);
		}
	};

	class Variable {
	public:
		String		name;
		size_t		offset   = 0;
		DataType	dataType = DataType::None;
		bool		rowMajor = true;

		template<class SE>
		void onJson(SE & se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, offset);
			SGE_NAMED_IO(se, dataType);
		}
	};

	class ConstBuffer {
	public:
		String			name;
		i16				bindPoint = 0;
		i16				bindCount = 0;
		size_t			dataSize  = 0;
		Vector<Variable, 4>	variables;

		template<class SE>
		void onJson(SE & se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, bindPoint);
			SGE_NAMED_IO(se, bindCount);
			SGE_NAMED_IO(se, dataSize);
			SGE_NAMED_IO(se, variables);
		}

		const Variable* findVariable(StrView propName) const {
			for (auto& v : variables) {
				if (v.name == propName) return &v;
			}
			return nullptr;
		}
	};

	Vector<Input, 8>		inputs;
	Vector<Param, 8>		params;
	Vector<ConstBuffer, 4>	constBuffers;

//----------
	class Texture {
	public:
		String		name;
		i16			bindPoint = 0;
		i16			bindCount = 0;
		DataType	dataType = DataType::None;

		template<class SE>
		void onJson(SE & se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, bindPoint);
			SGE_NAMED_IO(se, bindCount);
			SGE_NAMED_IO(se, dataType);
		}
	};
	Vector<Texture, 8>		textures;

//----------
	class Sampler {
	public:
		String		name;
		i16			bindPoint = 0;
		i16			bindCount = 0;
		RenderDataType	dataType = RenderDataType::None;

		template<class SE>
		void onJson(SE & se) {
			SGE_NAMED_IO(se, name);
			SGE_NAMED_IO(se, bindPoint);
			SGE_NAMED_IO(se, bindCount);
			SGE_NAMED_IO(se, dataType);
		}
	};
	Vector<Sampler, 8>		samplers;

//----------
	template<class SE>
	void onJson(SE & se) {
		SGE_NAMED_IO(se, profile);
		SGE_NAMED_IO(se, inputs);
		SGE_NAMED_IO(se, params);
		SGE_NAMED_IO(se, constBuffers);
		SGE_NAMED_IO(se, textures);
		SGE_NAMED_IO(se, samplers);
	}
};

}