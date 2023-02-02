#pragma once

#include "ShaderInfo.h"

namespace sge {

class Shader;

struct ShaderStage : public NonCopyable {
	const ShaderStageInfo* info() const { return &_info; }
protected:
	ShaderStageInfo _info;
};

struct ShaderVertexStage : public ShaderStage {
	static constexpr ShaderStageMask stageMask() { return ShaderStageMask::Vertex; }
};
struct ShaderPixelStage  : public ShaderStage {
	static constexpr ShaderStageMask stageMask() { return ShaderStageMask::Pixel; }
};

struct ShaderPass : public NonCopyable {
	ShaderPass(Shader* shader, ShaderInfo::Pass& info);

	virtual ~ShaderPass() = default;

	ShaderVertexStage*	vertexStage() { return _vertexStage; }
	ShaderPixelStage*	pixelStage()  { return _pixelStage;  } 
	const ShaderInfo::Pass*	info() const { return _info; }

protected:
	Shader* _shader = nullptr;
	ShaderInfo::Pass*  _info = nullptr;
	ShaderVertexStage* _vertexStage = nullptr;
	ShaderPixelStage*  _pixelStage  = nullptr;
};

class ShaderPermutations
{
public:
	using Info = ShaderInfo::Permutations;

	struct PermutationParam
	{
		using Info = ShaderInfo::Permutation;

		void create(const Info& info) {
			_info = &info; 
			_valueIdx = Math::clamp(_valueIdx, 0, static_cast<int>(info.values.size()));
		}
		void set(int valueIdx) { _valueIdx = valueIdx; }

		const Info* info() const { return _info; }

		StrView name()	const { return _info->name; }
		StrView value() const { return _info->values[_valueIdx]; }
		int valueIdx()	const { return _valueIdx; }

	protected:
		const Info* _info = nullptr;
		int			_valueIdx = 0;
	};

	void set(StrView name, StrView value)
	{
		int nameIdx		= -1;
		int valueIdx	= -1;

		if (!findIndex(nameIdx, valueIdx, name, value))
			throw Error(SGE_LOC, "unexpected permutation name/value");

		SGE_ASSERT(nameIdx < _permutParams.size());
		_permutParams[nameIdx].set(valueIdx);
	}

	void create(const Info& info)
	{
		_info = &info;
		_permutParams.resize(info.size());
		for (size_t i = 0; i < _permutParams.size(); i++)
		{
			_permutParams[i].create(info[i]);
		}
	}

	bool operator==(const ShaderPermutations& permuts) const
	{
		SGE_ASSERT(_permutParams.size() == permuts.size(), "wrong ShaderPermutations size");
		if (_permutParams.size() == 0)
			return false;
		for (size_t i = 0; i < _permutParams.size(); i++)
		{
			if (_permutParams[i].valueIdx() != permuts._permutParams[i].valueIdx())
				return false;
		}
		return true;
	}
	bool operator!=(const ShaderPermutations& permuts) const { return !(operator==(permuts)); }

	template<class STR>
	void nameTo(STR& o) const
	{
		//o.clear();
		TempString tmp;
		tmp.append("_");
		for (size_t i = 0; i < _permutParams.size(); i++)
		{
			auto& permut = _permutParams[i];
			StringUtil::toString(tmp, permut.valueIdx());
			o.append(tmp.c_str());
		}
	}

	void clear() { _permutParams.clear(); }

	const PermutationParam& operator[](size_t i) const { SGE_ASSERT(isInBoundary(i), "outside boundary"); return _permutParams[i]; }

	size_t size() const			{ return _permutParams.size(); }
	const Info* info() const	{ return _info; }

protected:
	bool findIndex(int& nameIdx, int& valueIdx, StrView name, StrView value)
	{
		SGE_ASSERT(_info, "permutation info");
		nameIdx		= -1;
		valueIdx	= -1;

		for (int i = 0; i < _info->size(); i++)
		{
			if ((*_info)[i].name == name)
				nameIdx = i;
		}
		if (nameIdx == -1)
			return false;

		auto& values = (*_info)[nameIdx].values;

		for (int i = 0; i < values.size(); i++)
		{
			if (values[i] == value)
				valueIdx = i;
		}
		if (valueIdx == -1)
			return false;

		return true;
	}
	bool isInBoundary(size_t i) const { return i >= 0 && i < _permutParams.size(); }

private:
	const Info*		_info = nullptr;
	Vector<PermutationParam, 2> _permutParams;
};

class Shader : public RefCountBase {
public:
	using Permutations = ShaderPermutations;

	Shader(StrView filename);
	Shader(StrView filename, const Permutations& permuts);

	virtual ~Shader();

	virtual void reset();

	const String& filename() const				{ return _filename; }

	const ShaderInfo* info() const				{ return &_info; }
	
	Span<UPtr<ShaderPass>>	passes()			{ return _passes; }

	const String& shadername() const			{ return _filename; }
	const	Permutations& permutations() const	{ return _permuts; }
			Permutations& permutations() 		{ return _permuts; }

protected:
	virtual void onReset() = 0;

protected:
	String	_filename;
	String	_shadername;
	ShaderInfo	_info;
	Vector<UPtr<ShaderPass>, 2> _passes;
	Permutations _permuts;
};

}