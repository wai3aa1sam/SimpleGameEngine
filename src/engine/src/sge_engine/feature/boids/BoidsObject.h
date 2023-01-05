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

struct BoidsData
{
	void setForward(const Vec3f& forward)
	{
		_forward = forward;
		//_rot = Quat4f::s_fromToRotation(Vec3f::s_forward(), forward);
	}

	Mat4f modelMatrix(const Vec3f& scale = Vec3f(1.0, 1.0, 1.0))
	{
		auto rot = Quat4f::s_fromToRotation(Vec3f::s_forward(), _forward);
		return Mat4f::s_TRS(_position, rot, scale);
	}

	Quat4f rotation() const { return Quat4f::s_fromToRotation(Vec3f::s_forward(), _forward); }

	Vec3f _position	= Vec3f::s_zero();
	Vec3f _forward	= Vec3f::s_zero();
	float _speed	= 0;

};

struct BoidsObjUpdateData
{
	Vec3f _position	= Vec3f::s_zero();
	Vec3f _forward	= Vec3f::s_zero();
	float _speed	= 0;

	Quat4f rotation() const { return Quat4f::s_fromToRotation(Vec3f::s_forward(), _forward); }

	//int	  nearbyObjCount		= 0;

	int	  separationCount		= 0;
	int	  alignmentCount		= 0;
	int	  cohesionCount			= 0;

	Vec3f totalSeparationPos	= Vec3f::s_zero();
	Vec3f totalAlignmentDir		= Vec3f::s_zero();
	Vec3f totalCohesionCenter	= Vec3f::s_zero();
};

class BoidsObjectManager
{
public:
	void init(Boids* boids);

	void setup();
	void update();
	void render(RenderRequest& rdReq, const Boids* boids);

	Span<BoidsObjUpdateData> getUpdateData()	{ return _updateData; }
	Span<BoidsData>			 data()				{ return _data; }

private:

	Vector<BoidsObjUpdateData>		_updateData;
	Vector<BoidsData>				_data;
	Vector<SPtr<Material>>			_mtls;	// useless if using instancing
};

}