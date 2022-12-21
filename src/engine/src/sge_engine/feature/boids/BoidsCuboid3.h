#pragma once

namespace sge {

class BoidsObject;

class BoidsCell3
{
public:
	using Vec3 = Vec3f;

	using Object = BoidsObject;
	using Objects = Vector<Object*>;

	Vec3i	idx;
	Objects objs;
};

class BoidsCuboid3
{
public:
	using T = float;
	using Vec3 = Vec3f;

	using Cell		= BoidsCell3;
	using Object	= Cell::Object;
	using Objects	= Cell::Objects;

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

	void getNearbyObjs(Objects& out, const Vec3& pos)
	{
		out.clear();

		auto* target = cellByPos(pos);
		if (!target)
			return;

		_getNearbyObjs(out, target, -1);
		_getNearbyObjs(out, target,  0);
		_getNearbyObjs(out, target, +1);
	}

	bool isValid() const		{ return _cells.size() > 0; }
	
	Span<BoidsCell3> cells()	{ return _cells; }

private:
	void _getNearbyObjs(Objects& out, Cell* target, int n)
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
	Vector<BoidsCell3> _cells;
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