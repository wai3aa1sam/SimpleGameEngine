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

void BoidsObjectManager::init(Boids* boids)
{
	auto& setting = boids->_setting;
	setting.objectCount = boids->roundupToMultiple(setting.objectCount, logicalThreadCount());

	_updateData.resize(setting.objectCount);
	_data.resize(setting.objectCount);
	_mtls.resize(setting.objectCount);

	auto* renderer = Renderer::instance();

	auto& shader	= boids->_shader;
	auto& texture	= boids->_texture;

	std::default_random_engine _rng{ std::random_device{}() };

	for (size_t i = 0; i < setting.objectCount; i++)
	{
		auto& obj = _data[i];

		auto forward = Physics::unitSphere<float>();
		
		obj.setForward(Physics::unitSphere<float>());
		obj._position = setting.boidsPos + Physics::unitSphere<float>() * setting.spawnRadius;
		//obj._velocity = obj->forward() * (setting.minSpeed + setting.maxSpeed) / 2.0;
	}

	for (size_t i = 0; i < setting.objectCount; i++)
	{
		auto& mtl = _mtls[i];

		mtl = renderer->createMaterial();
		mtl->setShader(shader);
		mtl->setParam("test_color",			Color4f(1, 1, 1, 1));
		mtl->setParam("mainTex",			texture);
		mtl->setParam("_time_scale",		1.0f);

		mtl->setParam("_speed",				Physics::random<float>());
		mtl->setParam("_frequency",			setting._frequency);
		mtl->setParam("_amplitude",			setting._amplitude);
		mtl->setParam("_headLimit",			setting._headLimit);

#if SGE_IS_FISH_ANIMATE_IMPL2
		// impl2
		mtl->setParam("_side_to_side",		_side_to_side);
		mtl->setParam("_pivot",			    _pivot);
		mtl->setParam("_wave",				_wave);
		mtl->setParam("_twist",			    _twist);
		mtl->setParam("_mask_black",		_mask_black);
		mtl->setParam("_mask_white",		_mask_white);
#endif // SGE_IS_FISH_ANIMATE_IMPL2

	}
	
	setup();
}

void BoidsObjectManager::setup()
{
	for (size_t i = 0; i < _data.size(); i++)
	{
		auto& readData   = _data[i];
		auto& updateData = _updateData[i];

		updateData._position = readData._position;
		updateData._forward  = readData._forward;
	}
}

void BoidsObjectManager::render(RenderRequest& rdReq, const Boids* boids)
{
	const auto& setting = boids->_setting;
	if (setting.isDrawMesh)
	{
		for (size_t i = 0; i < _data.size(); i++)
		{
			auto& data = _data[i];
			auto& mtl  = _mtls[i];

			mtl->setParam("sge_time", boids->_time);
			//obj->_mtl->setParam("_speed",				obj->_speed);

			rdReq.drawMesh(SGE_LOC, boids->_meshAsset->mesh, mtl, data.modelMatrix(setting._objScale));
		}
	}
	else
	{
		for (size_t i = 0; i < _data.size(); i++)
		{
			auto& data = _data[i];
			auto& mtl  = _mtls[i];

			mtl->setParam("sge_time", boids->_time);
			mtl->setParam("_speed",				10.0f);

			rdReq.drawLine(data._position, data._position + data._forward * setting.lineLength);
		}
	}

}

}