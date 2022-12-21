#include <sge_engine-pch.h>
#include "Boids.h"
#include "BoidsObstacle.h"
#include "BoidsObject.h"

#include <sge_core/multi_thread/Atomic.h>

namespace sge {

BoidsObstacleManager* Singleton<BoidsObstacleManager>::_instance = nullptr;

#if 1
template<> inline
const TypeInfo* TypeOf<Boids>() {
	using This = Boids;
	using TI_Base = TypeInfoInitNoBase<This>;

	class TI : public TI_Base {
	public:
		TI()
			: TI_Base("Boids")
		{
			static FieldInfo fi[] = {
				{"_setting",	&This::_setting },

				#if SGE_IS_FISH_ANIMATE_IMPL2
				{"_side_to_side",	&This::_side_to_side },
				{"_pivot",			&This::_pivot },
				{"_wave",			&This::_wave },
				{"_twist",			&This::_twist },
				{"_mask_black",		&This::_mask_black },
				{"_mask_white",		&This::_mask_white },
				#endif // 0
				
				#if SGE_IS_BOIDS_DEBUG

				{"_forward",	&This::_forward},
				{"_position",	&This::_position},
				#endif // SGE_IS_BOIDS_DEBUG

			};
			setFields(fi);
		}
	};
	static TI ti;
	return &ti;
}

const TypeInfo* CBoids::s_getType() {
	class TI : public TI_Base {
	public:
		TI() {
			static FieldInfo fi[] = {
				{"_boids",	&This::_boids	},
			};
			setFields(fi);
		}
	};
	static TI ti;
	return &ti;

}
#endif // 1

void Boids::start()
{
	std::uniform_real_distribution<float> urd(0.0f, _setting.spawnRadius);
	auto* renderer = Renderer::instance();

	{
		{
			Texture2D_CreateDesc texDesc;
			auto& image = texDesc.imageToUpload;

			//image.loadFile("Assets/Textures/uvChecker_BC7.dds");
			//image.loadFile("Assets/Textures/uvChecker.png");
			image.loadFile("Assets/boids/fish_texture.png");

			texDesc.size = image.size();
			texDesc.colorType = image.colorType();

			_texture = renderer->createTexture2D(texDesc);
		}

		{
			EditMesh editMesh;
			
			WavefrontObjLoader::loadFile(editMesh, "Assets/boids/fish2.obj");
			//WavefrontObjLoader::loadFile(editMesh, "Assets/Mesh/cone2.obj");

			// the current shader need color
			editMesh.addColors(Color4b(255, 255, 255, 255));

			_meshAsset = new MeshAsset();
			_meshAsset->mesh.create(editMesh);
		}

		{
			_shader = renderer->createShader("Assets/boids/fish_animation.shader");
		}
	}

	{
		_objs.resize(_setting.objectCount);
		for (size_t i = 0; i < _objs.size(); i++)
		{
			auto& obj = _objs[i];
			obj = new BoidsObject();
			obj->init(this);

			std::default_random_engine _rng{ std::random_device{}() };

			obj->setForward(Physics::unitSphere<T>());
			obj->_position = _setting.boidsPos + Physics::unitSphere<T>() * _setting.spawnRadius;
			obj->_velocity = obj->forward() * (_setting.minSpeed + _setting.maxSpeed) / 2.0;

			obj->_mtl = renderer->createMaterial();
			obj->_mtl->setShader(_shader);
			obj->_mtl->setParam("test_color", Color4f(1, 1, 1, 1));
			obj->_mtl->setParam("mainTex", _texture);
			obj->_mtl->setParam("_time_scale",			1.0f);

			obj->_mtl->setParam("_speed",				obj->_speed);
			obj->_mtl->setParam("_frequency",			_setting._frequency);
			obj->_mtl->setParam("_amplitude",			_setting._amplitude);
			obj->_mtl->setParam("_headLimit",			_setting._headLimit);

#if SGE_IS_FISH_ANIMATE_IMPL2
			// impl2
			obj->_mtl->setParam("_side_to_side",		_side_to_side);
			obj->_mtl->setParam("_pivot",			    _pivot);
			obj->_mtl->setParam("_wave",				_wave);
			obj->_mtl->setParam("_twist",			    _twist);
			obj->_mtl->setParam("_mask_black",			_mask_black);
			obj->_mtl->setParam("_mask_white",			_mask_white);
#endif // SGE_IS_FISH_ANIMATE_IMPL2

		}
	}

	createWalls();
	//auto cellSize = Math::max(Math::max( Math::max(_setting.minCellSize, _setting.separationRadius), _setting.alignmentRadius), _setting.cohesionRadius);
	auto cellSize = Math::max({ _setting.minCellSize, _setting.separationRadius, _setting.alignmentRadius, _setting.cohesionRadius });
	_setting.cellSize = Vec3f(cellSize, cellSize, cellSize);
	_cuboid.create(_setting.boidsSize, _setting.cellSize, _setting.boidsPos);
}

void Boids::update()
{
	_setting.alignmentViewAngle = Math::cos(Math::radians(_setting.alignmentViewAngle * 0.5f) );
	float dt = 1.0f / 60.0f; (void)dt;
	_time.x += dt;
	_time.y += dt;

	_cuboid.clear();

	for (size_t i = 0; i < _objs.size(); i++)
	{
		auto& obj = _objs[i];

#if SGE_IS_USE_CELL
		_cuboid.addObj(obj, obj->_position);
#endif // 0

	}

#if 0
	bool endLoop = false;
	for (size_t i = 0; i < _objs.size(); i += _setting.alternativeUpdate * 2)
	{
		for (size_t offset = 0; offset < _setting.alternativeUpdate; offset++)
		{
			auto index = i + offset;
			if (index > _objs.size())
			{
				endLoop = true;
				break;
			}
			auto& obj = _objs[index];
			auto& nearbyObjs = _objs;

			think(*obj, dt, nearbyObjs);
		}
		if (endLoop)
			break;
	}
#else
	auto& altUpdateIdx = _setting.alternativeUpdateIndex;
	altUpdateIdx = (altUpdateIdx + 1) % _setting.alternativeUpdate;

	for (size_t i = altUpdateIdx; i < _objs.size(); i += _setting.alternativeUpdate)
	{
		auto& obj = _objs[i];
		auto* nearbyObjs = &_objs;

#if SGE_IS_USE_CELL
		if (_setting.isUseCuboid)
		{
			_cuboid.getNearbyObjs(_tempObjs, obj->_position);
			nearbyObjs = &_tempObjs;
		}
#endif // 0

		think(*obj, dt, *nearbyObjs);
	}
#endif // 0

	for (size_t i = 0; i < _objs.size(); i++)
	{
		auto& obj = _objs[i]; (void)obj;
		obj->update();
	}

#if SGE_IS_BOIDS_DEBUG
	_objs[0]->_position = _position;
	_objs[0]->setForward(_forward);
#endif // SGE_IS_BOIDS_DEBUG

}

void Boids::render(RenderRequest& rdReq)
{
	_rdReq = &rdReq;
	rdReq.debug.drawBoundingBox = _setting.isDrawObjBoundingBox;

	Physics::Random rnd;

	for (size_t i = 0; i < _objs.size(); i++)
	{
		auto& obj = _objs[i];

		obj->_mtl->setParam("sge_time", _time);

		obj->_mtl->setParam("_time_scale",			_setting._time_scale);
		obj->_mtl->setParam("_headLimit",			_setting._headLimit);

#if 0
		T freq = remapAnimationFreq(obj->_speed);
		T amp  = remapAnimationAmp(obj->_speed);

		obj->_mtl->setParam("_speed",				remapAnimationSpeed(obj->_speed));
		obj->_mtl->setParam("_frequency",			Vec3{freq, freq, freq});
		obj->_mtl->setParam("_amplitude",			Vec3{amp, amp, amp});
#else
		obj->_mtl->setParam("_speed",				obj->_speed);
		obj->_mtl->setParam("_frequency",			_setting._frequency);
		obj->_mtl->setParam("_amplitude",			_setting._amplitude);
#endif // 0


#if SGE_IS_FISH_ANIMATE_IMPL2
		obj->_mtl->setParam("_side_to_side",		_side_to_side);
		obj->_mtl->setParam("_pivot",			    _pivot);
		obj->_mtl->setParam("_wave",				_wave);
		obj->_mtl->setParam("_twist",			    _twist);
		obj->_mtl->setParam("_mask_black",			_mask_black);
		obj->_mtl->setParam("_mask_white",			_mask_white);
#endif // 0

		rdReq.drawMesh(SGE_LOC, _meshAsset->mesh, obj->_mtl, obj->modelMatrix(_setting._objScale));
	}

#if 0
	{
		static float dt = 0.0f;
		dt += 1 / 60.0f;
		if (dt > 1)
		{
			SGE_LOG("=== start position");

			for (size_t i = 0; i < _objs.size(); i++)
			{
				auto& obj = _objs[i];
				auto idx = _cuboid.cellByPos(obj->_position)->idx;
				SGE_LOG("obj[{}]: _position: {}, cell: [{}, {}, {}]", i, obj->_position, idx.x, idx.y, idx.z);
			}

			SGE_LOG("=== end position");
			dt = 0;
		}
	}
#endif // 0

	if (_setting.isDrawObstacle)
	{
		for (size_t i = 0; i < _obstacles.size(); i++)
		{
			auto& obs = _obstacles[i];
			rdReq.drawBoundingBox(obs->_bbox);
		}
	}

	if (_setting.isDrawCells)
	{
		rdReq.drawBoundingBox(_setting.boidsPos, _setting.boidsSize / 2.0);
		_cuboid.render(rdReq);
	}
}

void Boids::think(BoidsObject& obj, float dt, Vector<BoidsObject*>& nearbyObjs)
{
	auto separationPos = Vec3f::s_zero();
	auto alignmentDir  = Vec3f::s_zero();
	auto cohesionPos   = Vec3f::s_zero();

	int separationCount = 0;
	int alignmentCount  = 0;
	int cohesionCount   = 0;

	auto accel  = Vec3f::s_zero();
	auto newVel = Vec3f::s_zero();

	const auto& mass = obj._mass; SGE_UNUSED(mass);

	for (auto& nearObj : nearbyObjs)
	{
		if (&obj == nearObj)
			continue;

		auto center_offset = nearObj->_position - obj._position;
		auto dist = center_offset.magnitude();
		center_offset /= dist;

		if (dist < _setting.separationRadius)
		{
			separationPos -= center_offset;
			separationCount++;
		}

		if (dist < _setting.alignmentRadius)
		{
			//if (center_offset.dot(aglinmentVel) > _setting.alignmentViewAngleCos)
			alignmentDir += nearObj->forward();
			alignmentCount++;
		}

		if (dist < _setting.cohesionRadius)
		{
			cohesionPos += nearObj->_position;
			cohesionCount++;
		}
	}

	if (separationCount)
	{
		auto seperationForce = steerForce(obj, separationPos / static_cast<T>(separationCount))	* _setting.seperateWeight;
		accel += seperationForce / mass;
	}

	if (alignmentCount)
	{
		auto alignmentForce  = steerForce(obj, alignmentDir / static_cast<T>(alignmentCount))	* _setting.alignmentWeight;
		accel += alignmentForce / mass;
	}

	if (cohesionCount)
	{
		auto cohesionForce   = steerForce(obj, cohesionPos / static_cast<T>(cohesionCount))		* _setting.cohesionWeight;
		accel += cohesionForce / mass;
	}

	Physics::RaycastResult3f hit;
	if (raycast(hit, obj._position, obj.forward(), _setting.avoidCollisionRadius))
	{
		auto forward = obj.forward();
		auto avoidDir = avoidObstacleRay(obj._position, forward, obj.rotation());
		auto avoidCollisionForce = steerForce(obj, avoidDir) * _setting.avoidCollisionWeight;
		accel += avoidCollisionForce / mass;

		//accel -= accel * 2;
		//obj._velocity = Vec3f::s_zero() - obj._velocity * 2.0;
		//obj.setForward(obj._velocity);
	}

	
#if SGE_IS_BOIDS_DEBUG

#else

	auto& vel = obj._velocity;
	vel += accel * dt;
	float speed = vel.magnitude() + Math::epsilon<T>();
	Vec3f dir = vel / speed;
	speed = Math::clamp(speed, _setting.minSpeed, _setting.maxSpeed);
	vel = dir * speed;

	obj._velocity = vel;
	obj.setForward(dir);
	obj._speed = speed;

	obj._position += obj._velocity * dt;

	if (!boundPos(obj._position))
	{
		//obj._velocity = Vec3f::s_zero() - obj._velocity;
		obj._position = _setting.boidsPos;
		obj.setForward(Physics::unitSphere<T>());
	}

#endif // SGE_IS_BOIDS_DEBUG

}

Vec3f Boids::steerForce(const BoidsObject& obj, const Vec3f& v)
{
	Vec3f ret = v.normalize() * _setting.maxSpeed - obj._velocity;
	return ret.clampMag(_setting.maxSteerForce);
}

Boids::Vec3 Boids::avoidObstacleRay(const Vec3& origin, const Vec3& forward, const Quat4& curRot)
{
	//using T = float;
	//using Vec3  = Vec3<T>;
	//using Quat4 = Quat4<T>;

	Physics::RaycastResult3<T> ret;

	for (auto& ray : _boidsRays.directions)
	{
		Vec3 dir =  curRot * ray;

		if (_rdReq)
		{
			if (_setting.isDrawRay)
			{
				_rdReq->drawLine(origin, origin + dir * _setting.avoidCollisionRadius, Color4b(255, 255, 255, 255));
			}
		}

		if (!raycast(ret, origin, dir, _setting.avoidCollisionRadius))
		{
			return dir;
		}
	}

	return forward;
}


bool Boids::boundPos(const Vec3f& pos)
{
	const auto  halfSize = _setting.boidsSize / T(2);
	const auto& boidsPos = _setting.boidsPos;

	if (pos.x < (boidsPos.x - halfSize.x) || pos.x > (boidsPos.x + halfSize.x)) { return false; }
	if (pos.y < (boidsPos.y - halfSize.y) || pos.y > (boidsPos.y + halfSize.y)) { return false; }
	if (pos.z < (boidsPos.z - halfSize.z) || pos.z > (boidsPos.z + halfSize.z)) { return false; }

	if (pos.x < (boidsPos.x - halfSize.x)) { return false; }
	if (pos.x > (boidsPos.x + halfSize.x)) { return false; }

	if (pos.y < (boidsPos.y - halfSize.y)) { return false; }
	if (pos.y > (boidsPos.y + halfSize.y)) { return false; }

	if (pos.z < (boidsPos.z - halfSize.z)) { return false; }
	if (pos.z > (boidsPos.z + halfSize.z)) { return false; }


	return true;
}

void Boids::createWalls()
{
	Vec3f pts[8];
	auto min_pt = Vec3f(-1.0f, -1.0f, -1.0f);
	auto max_pt = Vec3f( 1.0f,  1.0f,  1.0f);
	BBox3f bbox = {min_pt, max_pt};
	
	auto halfSize = _setting.boidsSize / 2.0;

	constexpr int wallCount = 6;
	_obstacles.reserve(wallCount);

	for (int i = 0; i < wallCount; i++)
	{
		auto& obs = _obstacles.emplace_back(new BoidsObstacle());
		
		auto add_sub_idx = i % 2;
		auto xyz_idx = i / 2;
		auto translate = Vec3f::s_zero();
		translate[xyz_idx] = (add_sub_idx == 0) ? translate[xyz_idx] - _setting.boidsSize[xyz_idx] : translate[xyz_idx] + _setting.boidsSize[xyz_idx];

		bbox.getPoints(pts, Mat4f::s_TS(translate + _setting.boidsPos, halfSize * T(1.1)));
		BBox3f::create(obs->_bbox, pts);
	}
}

}