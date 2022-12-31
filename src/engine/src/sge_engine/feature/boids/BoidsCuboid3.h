#pragma once

#include "BoidsObject.h"

namespace sge {

class BoidsObject;
struct BoidsData;

template<class OBJ>
class BoidsCell3
{
public:
	using Vec3 = Vec3f;

	using Object = OBJ;
	using Objects = Vector<Object*>;

	Vec3i	idx;
	Objects objs;
};

template<class OBJ>
class BoidsCuboid3
{
public:
	using T = float;
	using Vec3 = Vec3f;

	using Cell		= BoidsCell3<OBJ>;
	using Object	= typename Cell::Object;
	using Objects	= typename Cell::Objects;

	BoidsCuboid3() = default;
	BoidsCuboid3(const Vec3& cuboidSize, const Vec3& cellSize, const Vec3& position = {0, 0, 0})
	{
		create(cuboidSize, cellSize, position);
	}
	
	void reset()
	{
		_cells.clear();
	}

	void create(const Vec3& cuboidSize, const Vec3& cellSize, const Vec3& position = {0, 0, 0})
	{
		reset();
		_cuboidSize = cuboidSize;
		_cellSize   = cellSize;
		_position = position;

		auto count = cuboidSize / cellSize;
		_count = Vec3i(Math::ceilToInt(count.x), Math::ceilToInt(count.y), Math::ceilToInt(count.z));
		_cells.resize(_count.x * _count.y * _count.z);

		for (int z = 0; z < _count.z; z++)
		{
			for (int y = 0; y < _count.y; y++)
			{
				for (int x = 0; x < _count.x; x++)
				{
					_cells[x + _count.x * (y + _count.y * z)].idx.set(x, y, z);
				}
			}
		}
	}

	void clear()
	{
		for (auto& c : _cells)
		{
			c.objs.clear();
		}
	}

	Cell* cell(const Vec3i& idx) { return cell(idx.x, idx.y, idx.z); }

	Cell* cell(int x, int y, int z)
	{
		SGE_ASSERT(isValid());
		if (x < 0 || x >= _count.x) return nullptr;
		if (y < 0 || y >= _count.y) return nullptr;
		if (z < 0 || z >= _count.z) return nullptr;

		return &_cells[x + _count.x * (y + _count.y * z)];
	}

	Cell* cellByPos(const Vec3& pos)
	{
		SGE_ASSERT(isValid());
		auto offset = _cuboidSize / 2;
		auto idx = (pos + offset - _position) / _cellSize;
		return cell(Math::floorTo_Int(idx.x), Math::floorTo_Int(idx.y), Math::floorTo_Int(idx.z));
	}

	const Cell* cell(const Vec3i& idx) const { return cell(idx.x, idx.y, idx.z); }
	const Cell* cell(int x, int y, int z) const
	{
		SGE_ASSERT(isValid());
		if (x < 0 || x >= _count.x) return nullptr;
		if (y < 0 || y >= _count.y) return nullptr;
		if (z < 0 || z >= _count.z) return nullptr;

		return &_cells[x + _count.x * (y + _count.y * z)];
	}

	const Cell* cellByPos(const Vec3& pos) const
	{
		SGE_ASSERT(isValid());
		auto offset = _cuboidSize / 2;
		auto idx = (pos + offset - _position) / _cellSize;
		return cell(Math::floorTo_Int(idx.x), Math::floorTo_Int(idx.y), Math::floorTo_Int(idx.z));
	}

	void addObj(Object* obj, const Vec3& pos)
	{
		auto* cell = cellByPos(pos);
		if (cell)
		{
			cell->objs.emplace_back(obj);
		}
	}

	void render(RenderRequest& rdReq)
	{
		auto offset		  = _cuboidSize / 2;
		auto halfCellSize = _cellSize / 2;
		for (auto& cell : _cells)
		{
			auto pos = Vec3f::s_cast(cell.idx) * _cellSize - offset + halfCellSize + _position;

			if (cell.objs.size())
			{
				rdReq.drawBoundingBox(pos, halfCellSize);
			}
		}
		
#if 0
		static float dt = 0.0f;
		dt += 1.0f / 60.0f;
		if (dt > 1.0f)
		{
			SGE_LOG("=== start log");

			for (auto& cell : _cells)
			{
				if (cell.objs.size() > 0)
				{
					SGE_LOG("cell[{}, {}, {}] = {} objects", cell.idx.x, cell.idx.y, cell.idx.z, cell.objs.size());
				}
			}

			SGE_LOG("=== end log");
			dt = 0.0f;
		}
#endif // 0
	}

	void getNearbyObjs(Objects& out, const Vec3& pos) const
	{
		out.clear();

		const auto* target = cellByPos(pos);
		if (!target)
			return;

		_getNearbyObjs(out, target, -1);
		_getNearbyObjs(out, target,  0);
		_getNearbyObjs(out, target, +1);
	}

	bool isValid() const		{ return _cells.size() > 0; }
	
	Span<Cell> cells()	{ return _cells; }

private:
	void _getNearbyObjs(Objects& out, const Cell* target, int n) const
	{
		{ auto c = cell(target->idx.x - 1, target->idx.y + n, target->idx.z - 1); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x    , target->idx.y + n, target->idx.z - 1); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x + 1, target->idx.y + n, target->idx.z - 1); if (c) { out.appendRange(c->objs); } }

		{ auto c = cell(target->idx.x - 1, target->idx.y + n, target->idx.z    ); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x    , target->idx.y + n, target->idx.z    ); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x + 1, target->idx.y + n, target->idx.z    ); if (c) { out.appendRange(c->objs); } }

		{ auto c = cell(target->idx.x - 1, target->idx.y + n, target->idx.z + 1); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x    , target->idx.y + n, target->idx.z + 1); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x + 1, target->idx.y + n, target->idx.z + 1); if (c) { out.appendRange(c->objs); } }
	}

private:
	Vector<Cell> _cells;
	Vec3i _count;
	Vec3  _cuboidSize;
	Vec3  _cellSize;
	Vec3  _position;
};


template<>
class BoidsCell3<BoidsData>
{
public:
	using Vec3 = Vec3f;
	using OBJ = BoidsData;

	using Object	= OBJ;
	using Objects	= Vector<Object>;

	Vec3i	idx;
	Objects objs;
};

template<>
class BoidsCuboid3<BoidsData>
{
public:
	using T = float;
	using Vec3 = Vec3f;

	using OBJ		= BoidsData;
	using Cell		= BoidsCell3<OBJ>;
	using Object	= typename Cell::Object;
	using Objects	= typename Cell::Objects;

	BoidsCuboid3() = default;
	BoidsCuboid3(const Vec3& cuboidSize, const Vec3& cellSize, const Vec3& position = {0, 0, 0})
	{
		create(cuboidSize, cellSize, position);
	}

	void reset()
	{
		_cells.clear();
	}

	void create(const Vec3& cuboidSize, const Vec3& cellSize, const Vec3& position = {0, 0, 0})
	{
		reset();
		_cuboidSize = cuboidSize;
		_cellSize   = cellSize;
		_position = position;

		auto count = cuboidSize / cellSize;
		_count = Vec3i(Math::ceilToInt(count.x), Math::ceilToInt(count.y), Math::ceilToInt(count.z));
		_cells.resize(_count.x * _count.y * _count.z);

		for (int z = 0; z < _count.z; z++)
		{
			for (int y = 0; y < _count.y; y++)
			{
				for (int x = 0; x < _count.x; x++)
				{
					_cells[x + _count.x * (y + _count.y * z)].idx.set(x, y, z);
				}
			}
		}
	}

	void clear()
	{
		for (auto& c : _cells)
		{
			c.objs.clear();
		}
	}

	Cell* cell(const Vec3i& idx) { return cell(idx.x, idx.y, idx.z); }

	Cell* cell(int x, int y, int z)
	{
		SGE_ASSERT(isValid());
		if (x < 0 || x >= _count.x) return nullptr;
		if (y < 0 || y >= _count.y) return nullptr;
		if (z < 0 || z >= _count.z) return nullptr;

		return &_cells[x + _count.x * (y + _count.y * z)];
	}

	Cell* cellByPos(const Vec3& pos)
	{
		SGE_ASSERT(isValid());
		auto offset = _cuboidSize / 2;
		auto idx = (pos + offset - _position) / _cellSize;
		return cell(Math::floorTo_Int(idx.x), Math::floorTo_Int(idx.y), Math::floorTo_Int(idx.z));
	}

	const Cell* cell(const Vec3i& idx) const { return cell(idx.x, idx.y, idx.z); }
	const Cell* cell(int x, int y, int z) const
	{
		SGE_ASSERT(isValid());
		if (x < 0 || x >= _count.x) return nullptr;
		if (y < 0 || y >= _count.y) return nullptr;
		if (z < 0 || z >= _count.z) return nullptr;

		return &_cells[x + _count.x * (y + _count.y * z)];
	}

	const Cell* cellByPos(const Vec3& pos) const
	{
		SGE_ASSERT(isValid());
		auto offset = _cuboidSize / 2;
		auto idx = (pos + offset - _position) / _cellSize;
		return cell(Math::floorTo_Int(idx.x), Math::floorTo_Int(idx.y), Math::floorTo_Int(idx.z));
	}

	void addObj(const Object& obj)
	{
		auto* cell = cellByPos(obj._position);
		if (cell)
		{
			cell->objs.emplace_back(obj);
		}
	}

	void render(RenderRequest& rdReq)
	{
		auto offset		  = _cuboidSize / 2;
		auto halfCellSize = _cellSize / 2;
		for (auto& cell : _cells)
		{
			auto pos = Vec3f::s_cast(cell.idx) * _cellSize - offset + halfCellSize + _position;

			if (cell.objs.size())
			{
				rdReq.drawBoundingBox(pos, halfCellSize);
			}
		}

#if 0
		static float dt = 0.0f;
		dt += 1.0f / 60.0f;
		if (dt > 1.0f)
		{
			SGE_LOG("=== start log");

			for (auto& cell : _cells)
			{
				if (cell.objs.size() > 0)
				{
					SGE_LOG("cell[{}, {}, {}] = {} objects", cell.idx.x, cell.idx.y, cell.idx.z, cell.objs.size());
				}
			}

			SGE_LOG("=== end log");
			dt = 0.0f;
		}
#endif // 0
	}

	void getNearbyObjs(Objects& out, const Vec3& pos) const
	{
		out.clear();

		const auto* target = cellByPos(pos);
		if (!target)
			return;

		_getNearbyObjs(out, target, -1);
		_getNearbyObjs(out, target,  0);
		_getNearbyObjs(out, target, +1);
	}

	bool isValid() const		{ return _cells.size() > 0; }

	Span<Cell> cells()	{ return _cells; }

private:
	void _getNearbyObjs(Objects& out, const Cell* target, int n) const
	{
		{ auto c = cell(target->idx.x - 1, target->idx.y + n, target->idx.z - 1); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x    , target->idx.y + n, target->idx.z - 1); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x + 1, target->idx.y + n, target->idx.z - 1); if (c) { out.appendRange(c->objs); } }

		{ auto c = cell(target->idx.x - 1, target->idx.y + n, target->idx.z    ); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x    , target->idx.y + n, target->idx.z    ); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x + 1, target->idx.y + n, target->idx.z    ); if (c) { out.appendRange(c->objs); } }

		{ auto c = cell(target->idx.x - 1, target->idx.y + n, target->idx.z + 1); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x    , target->idx.y + n, target->idx.z + 1); if (c) { out.appendRange(c->objs); } }
		{ auto c = cell(target->idx.x + 1, target->idx.y + n, target->idx.z + 1); if (c) { out.appendRange(c->objs); } }
	}

private:
	Vector<Cell> _cells;
	Vec3i _count;
	Vec3  _cuboidSize;
	Vec3  _cellSize;
	Vec3  _position;
};


#if 0
#pragma mark --- BoidsCuboid3-Impl ---
#endif // 0
#if 1



#endif

}