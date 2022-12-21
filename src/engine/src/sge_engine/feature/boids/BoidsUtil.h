#pragma once

#include "BoidsObstacle.h"
#include <random>

namespace sge {
namespace Physics{

template<class T>
inline T random()
{
	std::uniform_real_distribution<T> urd(0.0, 1.0 );
	std::default_random_engine _rng{ std::random_device{}() };
	return urd(_rng);
}

class Random
{
public:
	Random() = default;

	template<class T>
	T range(T min, T max)
	{
		std::uniform_real_distribution<T> urd(min, max);
		return urd(_rng);
	}

private:
	std::default_random_engine _rng{ std::random_device{}() };
};

template<class T> inline
Vec3f unitSphere()
{
	//using T = float;
	using Vec3 = Vec3<T>;

	Random rnd;

	T r		= rnd.range<T>(0.0, 1.0);
	T theta = rnd.range<T>(0.0, 1.0) * 2 * Math::PI<T>();
	T phi	= rnd.range<T>(0.0, 1.0) * Math::PI<T>();

	//r = Math::rsqrt_fast(r);
	r = r * r * r;

	auto sin_phi = Math::sin(phi);
	return Vec3(r * sin_phi * Math::cos(theta), r * Math::cos(phi), r * sin_phi * Math::sin(theta));
}

template<class T>
struct RaycastResult3
{
	using Vec3 = Vec3<T>;

	//float distance = -1.0f;				// ray origin to the impact point
	T t			= T(0.0);
	Vec3 point  = { 0.0f, 0.0f, 0.0f };
	Vec3 normal = { 0.0f, 0.0f, 0.0f };

	void reset()
	{
		t		= 0.0;
		point   = { 0.0f, 0.0f, 0.0f };
		normal  = { 0.0f, 0.0f, 0.0f };
	}
};
using RaycastResult3f = RaycastResult3<float>;

template<class T> inline
bool isHittedAABB(RaycastResult3<T>& out, const Vec3<T>& origin, const Vec3<T>& direction, T maxDist, const Vec3<T>& min_pt_, const Vec3<T>& max_pt_)
{
	using Vec3 = Vec3<T>;
	using RaycastResult = RaycastResult3<T>;

	RaycastResult& ret = out;
	ret.reset();

	Vec3 min_pt = min_pt_;
	Vec3 max_pt = max_pt_;

	auto dir_ray = direction + Math::epsilon<T>();		// avoid divide 0

	// p(t) = p0 + dir * t, 
	Vector<T, 6> ts; ts.resize(6);

	ts[0] = (min_pt.x - origin.x) / (dir_ray.x);		// t_min_x 
	ts[1] = (max_pt.x - origin.x) / (dir_ray.x);		// t_max_x 
	ts[2] = (min_pt.y - origin.y) / (dir_ray.y);		// t_min_y 
	ts[3] = (max_pt.y - origin.y) / (dir_ray.y);		// t_max_y 
	ts[4] = (min_pt.z - origin.z) / (dir_ray.z);		// t_min_z 
	ts[5] = (max_pt.z - origin.z) / (dir_ray.z);		// t_max_z 

	float t_min = Math::max(
		Math::max(Math::min(ts[0], ts[1]), Math::min(ts[2], ts[3])),
		Math::min(ts[4], ts[5])
	);

	float t_max = Math::min(
		Math::min(Math::max(ts[0], ts[1]), Math::max(ts[2], ts[3])),
		Math::max(ts[4], ts[5])
	);

	if (t_max < T(0.0) || t_min > t_max)		// no intersection
	{
		ret.t = t_max;
		return false;
	}

	if (t_min < T(0.0))							// inside aabb
		ret.t = t_max;
	else
		ret.t = t_min;

	if (ret.t > maxDist)
		return false;

	/*Vector<Vec3, 6> normals = {
		Vec3{-1.0,  0.0,  0.0}, Vec3{1.0, 0.0, 0.0},
		Vec3{ 0.0, -1.0,  0.0}, Vec3{0.0, 1.0, 0.0},
		Vec3{ 0.0,  0.0, -1.0}, Vec3{0.0, 0.0, 1.0},
	};*/
	Vector<Vec3, 6> normals;
	normals.emplace_back(Vec3{-1.0,  0.0,  0.0});
	normals.emplace_back(Vec3{ 1.0,  0.0,  0.0});
	normals.emplace_back(Vec3{ 0.0, -1.0,  0.0});
	normals.emplace_back(Vec3{ 0.0,  1.0,  0.0});
	normals.emplace_back(Vec3{ 0.0,  0.0, -1.0});
	normals.emplace_back(Vec3{ 0.0,  0.0,  1.0});

	for (int i = 0; i < normals.size(); i++)
	{
		if (Math::equals(ret.t, ts[i]))
			ret.normal = normals[i];
	}

	ret.point = origin + direction * ret.t;

	return true;
}

}
}