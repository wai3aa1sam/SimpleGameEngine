#pragma once
#include "BoidsSetting.h"
#include "BoidsCuboid3.h"
#include "BoidsObject.h"

#include "BoidsUtil.h"

#include "sge_engine/components/Component.h"

#include <sge_core/multi_thread/job_system/src/job_system.h>

// References:
// https://github.com/SebLague/Boids/tree/master
// 

namespace sge {

inline Atomic<size_t> kTest = 0;
inline Atomic<bool>   isAll = false;

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

	~Boids()
	{
		SGE_LOG("~Boids()");
		_objAllocator.destruct<BoidsObject>();
	}

	void start();
	void update();

	void render(RenderRequest& rdReq);

	void think(BoidsObject& obj, float dt, Vector<BoidsObject*>& nearbyObjs);

	Vec3f steerForce(const BoidsObject& obj, const Vec3f& v);
	Vec3f steerForce(const Vec3f& vel, const Vec3f& v);

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

	static size_t roundupToMultiple(size_t v, size_t n) { return (v + n - 1) / n * n; }

private:
	void createWalls();

	bool boundPos(const Vec3f& pos);

	Vec3 avoidObstacleRay(const Vec3& origin, const Vec3& forward, const Quat4& curRot);

#if SGE_IS_MT_BOIDS
	struct Buf
	{
		using Object = BoidsData;
		static constexpr size_t s_kBufCount = 16;
		Vector<Object> buf;
		Atomic<int>			 ownThread = -1;

		Buf()
		{
			ownThread.store(-1);
		}

		static Buf* getBufs()
		{
			static Buf bufs[Buf::s_kBufCount];
			return bufs;
		}

		static Buf& getBuf()
		{
			return getBufs()[threadLocalId()];
		}

		static Vector<Object>& getTmpObjBuffer()
		{
			if (getBuf().isOtherOwning())
			{
				throw SGE_ERROR("thread{} is owning, thread {} is attempt to get", getBuf().ownThread.load(), threadLocalId());
			}
			getBuf().ownThread.store(threadLocalId());
			return getBuf().buf;
		}

		static void freeOwnBuf()
		{
			getBuf().ownThread.store(-1);
		}

		bool isOtherOwning() const { return ownThread.load() != -1; }
	};
#endif // 0

public:


	RenderRequest* _rdReq = nullptr;

	BoidsObstacleManager _obsManager;

	BoidsSetting _setting;
	
	Vector<BoidsObstacle*> _obstacles;
	BoidsCuboid3<BoidsObject> _cuboid;
	Vector<BoidsObject*> _tempObjs;

	Vector<BoidsObject*> _objs;
	temp::LinearAllocator _objAllocator;

	//Vector<BoidsUpdateObject> _updateObjs;

	BoidsObjectManager _objManager;

	BoidsRays _boidsRays;

	SPtr<MeshAsset> _meshAsset;
	SPtr<Shader>	_shader;
	SPtr<Texture2D>	_texture;

	Vec4f _time = Vec4f{0, 0, 0, 0};

	bool _enableUpdate = false;
	bool _enableRender = false;

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


#if SGE_IS_MT_BOIDS

public:
	int _objDivSlicePerTime = 1;
	
private:

private:

	int _timeSliceIdx = 0;
	BoidsCuboid3<BoidsData> _cuboid_mt;

	class SetupObjJob : public JobParFor_Base<SetupObjJob>
	{
	public:
		void setup(Boids* boids_)
		{
			this->boids			= boids_;
			this->updateData	= boids_->_objManager.getUpdateData();
			this->readData		= boids_->_objManager.data();
		}

		void execute(const JobArgs& args)
		{
			SGE_PROFILE_SCOPED;

			auto id = args.loopIndex;

			auto& update		= updateData[id];
			const auto& read	= readData[id];

			update._position = read._position;
			update._forward  = read._forward;
			update._speed	 = read._speed;
		}

	private:
		Boids*						boids		= nullptr;

		Span<BoidsObjUpdateData>	updateData;
		Span<BoidsData>				readData;
	};

	class PreceieveNearObjJob : public JobParFor_Base<PreceieveNearObjJob>
	{
	public:
		void setup(Boids* boids_)
		{
			this->boids		= boids_;

			this->updateData	= boids_->_objManager.getUpdateData();
			this->readData		= boids_->_objManager.getUpdateData();

			#if SGE_IS_BOIDS_NO_NEAR
			this->nBoidsObj	= 0;
			#else
			this->nBoidsObj	= boids_->_objManager.getUpdateData().size();
			#endif // SGE_IS_BOIDS_NO_NEAR
		}

		void execute(const JobArgs& args)
		{
			SGE_PROFILE_SCOPED;

			auto id = args.loopIndex;

			auto& tempObjs = Buf::getTmpObjBuffer();
			//Vector<BoidsObject*> tempObjs;

			const auto& setting = boids->_setting;

			auto& target = updateData[id];

			auto& cuboid			= boids->_cuboid_mt;

			#if SGE_IS_USE_CELL
			cuboid.getNearbyObjs(tempObjs, target._position);
			auto nearObjSize = tempObjs.size();
			#else
			auto nearObjSize = data.nBoidsObj;
			#endif

			#if SGE_IS_BOIDS_NO_NEAR
			nearObjSize = 0;
			#endif // SGE_IS_BOIDS_NO_NEAR

			for (size_t iObj = 0; iObj < nearObjSize; iObj++)
			{
				if (id == iObj)
					continue;

				#if SGE_IS_USE_CELL
				const auto& nearObj = tempObjs[iObj];
				#else
				const auto& nearObj = readData[iObj];
				#endif // 0

				auto center_offset = nearObj._position - target._position;
				auto dist_sq = center_offset.sqrMagnitude();

				if (dist_sq < setting.separationRadius * setting.separationRadius)
				{
					target.totalSeparationPos -= center_offset / dist_sq;
					target.separationCount++;
				}

				if (dist_sq < setting.alignmentRadius * setting.alignmentRadius)
				{
					target.totalAlignmentDir += nearObj._forward;
					target.alignmentCount++;
				}

				if (dist_sq < setting.cohesionRadius * setting.cohesionRadius)
				{
					target.totalCohesionCenter += nearObj._position;
					target.cohesionCount++;
				}
			}
			Buf::freeOwnBuf();
		}

	private:
		Boids*						boids		= nullptr;

		size_t						nBoidsObj	= 0;
		Span<BoidsObjUpdateData>	updateData;
		Span<BoidsObjUpdateData>	readData;
	};

	class UpdateObjJob : public JobParFor_Base<UpdateObjJob>
	{
	public:
		void setup(Boids* boids_)
		{
			this->boids = boids_;

			this->readData		= boids_->_objManager.getUpdateData();
			this->updateData	= boids_->_objManager.data();
		}

		void execute(const JobArgs& args)
		{
			SGE_PROFILE_SCOPED;

			auto id = args.loopIndex;

			auto& update		= updateData[id];
			const auto& read	= readData[id];

			const auto& setting = boids->_setting;

			float dt = 1.0f / 60;

			auto separationCount = read.separationCount;
			auto alignmentCount  = read.alignmentCount;
			auto cohesionCount	 = read.cohesionCount;

			const auto& totalSeparationPos	= read.totalSeparationPos;
			const auto& totalAlignmentDir	= read.totalAlignmentDir;
			const auto& totalCohesionCenter = read.totalCohesionCenter;

			auto accel = Vec3f::s_zero();
			auto vel = read._forward * read._speed;
			float mass = 1.0f;

			if (separationCount)
			{
				auto seperationForce = boids->steerForce(vel, totalSeparationPos / static_cast<T>(separationCount))	* setting.seperateWeight;
				accel += seperationForce / mass;
			}

			if (alignmentCount)
			{
				auto alignmentForce  = boids->steerForce(vel, totalAlignmentDir / static_cast<T>(alignmentCount))	* setting.alignmentWeight;
				accel += alignmentForce / mass;
			}

			if (cohesionCount)
			{
				auto cohesionForce   = boids->steerForce(vel, totalCohesionCenter / static_cast<T>(cohesionCount))	* setting.cohesionWeight;
				accel += cohesionForce / mass;
			}

			Physics::RaycastResult3f hit;
			if (boids->raycast(hit, read._position, read._forward, setting.avoidCollisionRadius))
			{
				auto forward = read._forward;
				auto avoidDir = boids->avoidObstacleRay(read._position, forward, read.rotation());
				auto avoidCollisionForce = boids->steerForce(vel, avoidDir) * setting.avoidCollisionWeight;
				accel += avoidCollisionForce / mass;
			}

			vel += accel * dt;
			float speed = vel.magnitude() + Math::epsilon<T>();
			Vec3f dir = vel / speed;
			speed = Math::clamp(speed, setting.minSpeed, setting.maxSpeed);
			vel = dir * speed;

			update._forward = dir;
			update._position += vel * dt;
			update._speed = speed;

			if (!boids->boundPos(update._position))
			{
				//obj._velocity = Vec3f::s_zero() - obj._velocity;
				update._position = setting.boidsPos + Physics::unitSphere<float>() * setting.spawnRadius;
				update._forward  = Physics::unitSphere<T>();
			}
		}

	private:
		Boids*						boids		= nullptr;

		Span<BoidsObjUpdateData>	readData;
		Span<BoidsData>				updateData;
	};

	SetupObjJob			_setupObjJob;
	PreceieveNearObjJob _preceieveNearObjJob;
	UpdateObjJob		_updateObjJob;

	JobHandle _handle = nullptr;
	
#endif // 0

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