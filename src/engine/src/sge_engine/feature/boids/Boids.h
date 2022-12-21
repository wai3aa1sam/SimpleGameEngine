#pragma once
#include "BoidsSetting.h"
#include "BoidsCuboid3.h"

#include "BoidsUtil.h"

#include "sge_engine/components/Component.h"

#include <sge_core/multi_thread/job_system/src/job_system.h>


// References:
// https://github.com/SebLague/Boids/tree/master
// 

namespace sge {

class BoidsObject;
class BoidsObstacle;

class BoidsRays
{
public:
	using T = float;
	using Vec3 = Vec3<T>;

#if 0
	static constexpr int s_kDirections = 300;

	BoidsRays()
	{
		directions.reserve(s_kDirections);

		T goldenRatio = (1 + Math::rsqrt_fast(T(5.0))) / T(2.0);
		T angleIncrement = Math::PI<T>() * 2 * goldenRatio;

		for (int i = 0; i < s_kDirections; i++) {
			T t = (T) i / s_kDirections;
			T inclination = std::acos(1 - 2 * t);
			T azimuth = angleIncrement * i;

			T x = Math::sin(inclination) * Math::cos (azimuth);
			T y = Math::sin(inclination) * Math::sin (azimuth);
			T z = Math::cos(inclination);
			//directions[i] = Vec3(x, y, z);
			directions.emplace_back(Vec3(x, y, z));
		}
	}

#else

	static constexpr int s_kDirections = 20;
	BoidsRays()
	{
		directions.reserve(s_kDirections);

		Vec3 dirs[] = {
			Vec3(1,  1,  1), Vec3(1, -1,  1),	Vec3(-1, -1,  1),	Vec3(-1,  1,  1),
			Vec3(1,  1, -1), Vec3(1, -1, -1),	Vec3(-1, -1, -1),	Vec3(-1,  1, -1),
			Vec3(1,  1,  0), Vec3(1, -1,  0),	Vec3(-1, -1,  0),	Vec3(-1,  1,  0),
			Vec3(1,  0,  1), Vec3(-1,  0,  1),	Vec3(1,  0, -1),	Vec3(-1,  0, -1),
			Vec3(0,  1,  1), Vec3(0, -1,  1),	Vec3(0, -1, -1),	Vec3(0,  1, -1),
		};

		for (int i = 0; i < s_kDirections; i++)
		{
			directions.emplace_back(dirs[i].normalize());
		}
	}

#endif // 0

	Vector<Vec3> directions;
};

class Boids : public NonCopyable
{
	friend class BoidsObject;
public:
	using T = float;
	using Vec3	= Vec3<T>;
	using Quat4 = Quat4<T>;

	void start();
	void update();

	void render(RenderRequest& rdReq);

	void think(BoidsObject& obj, float dt, Vector<BoidsObject*>& nearbyObjs);

	Vec3f steerForce(const BoidsObject& obj, const Vec3f& v);

	bool raycast(Physics::RaycastResult3<T>& out, const Vec3& origin, const Vec3& direction, T maxDist)
	{
		for (auto& obs : _obstacles)
		{
			bool isHitted = Physics::isHittedAABB(out, origin, direction, maxDist, obs->_bbox.min, obs->_bbox.max);
			if (isHitted)
				return isHitted;
		}
		return false;
	}

private:
	void createWalls();

	bool boundPos(const Vec3f& pos);

	Vec3 avoidObstacleRay(const Vec3& origin, const Vec3& forward, const Quat4& curRot);

public:
	RenderRequest* _rdReq = nullptr;

	BoidsObstacleManager _obsManager;

	BoidsSetting _setting;
	Vector<BoidsObject*> _objs;
	Vector<BoidsObstacle*> _obstacles;
	BoidsCuboid3 _cuboid;

	Vector<BoidsObject*> _tempObjs;

	BoidsRays _boidsRays;

	SPtr<MeshAsset> _meshAsset;
	SPtr<Shader>	_shader;
	SPtr<Texture2D>	_texture;

	Vec4f _time = Vec4f{0, 0, 0, 0};

#if SGE_IS_FISH_ANIMATE_IMPL2
	float _side_to_side = 0;
	float _pivot = 0;
	float _wave = 0;
	float _twist = 0;
	float _mask_black = 0;
	float _mask_white = 0;
#endif // SGE_IS_FISH_ANIMATE2

#if SGE_IS_BOIDS_DEBUG
public:
	Vec3f _forward  = Vec3f(0, 0, 1);
	Vec3f _position = Vec3f(0, 0, 0);
private:
#endif // SGE_IS_BOIDS_DEBUG

private:
	JobSystem _jsys;
};

#if 1
class CBoids : public Component {
	SGE_OBJECT_TYPE(CBoids, Component)
public:
	CBoids() {}

	Boids _boids;
};

template<> const TypeInfo* TypeOf<Boids>();

#endif // 1


}