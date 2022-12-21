#include <sge_engine-pch.h>

#include "BoidsObject.h"
#include "BoidsObstacle.h"
#include "Boids.h"

namespace sge {

void BoidsObject::start()
{
}

void BoidsObject::update()
{
#if 0
	float dt = 1.0f / 60.0f;

	//_lastPosition = _position;
	//_lastVelocity = _velocity;

	auto position = _position;
	auto forward  = _forward;

	Physics::RaycastResult3f hit;
	if (_boids->raycast(hit, position, forward, _collisionRadius))
	{
		auto dot = forward.dot(hit.normal);

		if (Math::equals(dot, -T(1.0)))
		{
			forward -= forward * 2;
		}
		else
		{
			//forward = hit.normal * dot;
			forward = forward.reflect(hit.normal);
		}
		setForward(forward);
		//_velocity += _forward * _speed;
		//return;
	}

	_position += (_velocity * dt);
	//_position += this->forward() * (_speed * dt);
#endif // 0


}

void BoidsObject::init(Boids* p)
{
	_setting = &p->_setting;
	_boids = p;
}

}