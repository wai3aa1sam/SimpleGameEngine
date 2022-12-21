#pragma once
#include "BoidsUtil.h"
#include "BoidsSetting.h"
#include "../../assets/MeshAsset.h"

namespace sge {

class Boids;

class BoidsObject
{
	using T = float;
	using Setting = BoidsSetting;
	using Vec3  = Vec3<T>;
	using Quat4 = Quat4<T>;
public:
	void start();
	void update();

	void init(Boids* p);

	Mat4f modelMatrix(const Vec3& scale = Vec3(1.0, 1.0, 1.0))
	{
		//return Mat4f::s_translate(_position);
		return Mat4f::s_TRS(_position, _rot, scale);
	}

	void setForward(const Vec3f& forward)
	{
		_forward = forward.normalize();
		_rot = Quat4::s_fromToRotation(Vec3::s_forward(), _forward);
	}

	void setRotation(const Quat4& rot)
	{
		_rot = rot;
		_forward = _rot * _forward;
	}

	Vec3	forward() const		{ return _forward; }
	Quat4	rotation() const	{ return _rot; }

public:
	BoidsSetting* _setting = nullptr;
	Boids* _boids = nullptr;

	float _mass = 1.0f;
	float _speed = 10;

	Vec3f _position;
	Vec3f _velocity;

	SPtr<MeshAsset> _mesh;
	SPtr<Material>  _mtl;
private:
	Quat4f _rot = Quat4f::s_identity();
	Vec3f _forward = {0.0f, 0.0f, 1.0f};
};

}