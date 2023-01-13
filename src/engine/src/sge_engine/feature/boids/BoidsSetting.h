#pragma once

namespace sge {

#define SGE_IS_USE_CELL 1
#define SGE_IS_USE_ACCEL 1
#define SGE_IS_FISH_ANIMATE_IMPL2 0
#define SGE_IS_BOIDS_DEBUG 0

#define SGE_IS_BOIDS_NO_NEAR 0

#define SGE_IS_MT_BOIDS 1

class BoidsSetting : public NonCopyable
{
	using T = float;
	using Vec3 = Vec3<T>;
public:
	size_t objectCount = 10000;

	int batchSize = (int)objectCount / 16;

	// Debug
	bool isUseCuboid			= true;
	bool isDrawCells			= false;
	bool isDrawObstacle			= false;
	bool isDrawObjBoundingBox	= false;
	bool isDrawRay				= false;
	bool isDrawMesh				= false;

	float lineLength			= 1.5f;

	// Settings
	float minSpeed = 10;
	float maxSpeed = 15;
	//float nearRadius = 2.5f;
	//float avoidRadius = 1;
	float maxSteerForce = 3;
	float changeRate = 0.1f;
	float rndSteering   = 45;
	float rndSpeed		= 1;

	float targetWeight = 1;

	// cuboid
#if SGE_IS_BOIDS_DEBUG
	Vec3f boidsPos	   = { 0,  0, 0};
#else
	Vec3f boidsPos	   = { 0,  -10, -90 / 2};
#endif // SGE_IS_BOIDS_DEBUG

	float factor = 1.0;

	float spawnRadius	= 50 * factor;

	Vec3f boidsSize	   = Vec3f{spawnRadius, spawnRadius, spawnRadius} ;
	float minCellSize  = 1.0f * factor;
	float minBoidsSize = 10.0f;
	Vec3f cellSize	   ;//= {4.0, 4.0, 4.0};

	//Vec3i cellCount	  = {  10,   10, 10};

	//LayerMask obstacleMask;
	//float boundsRadius = .27f;
	//float avoidCollisionWeight = 10;
	//float collisionAvoidDst = 5;

	// --- Boids param
	float separationRadius = 1;
	float seperateWeight = 1;

	float alignmentRadius = 1;
	float alignmentWeight = 1;
	float alignmentViewAngle = 120;
	float alignmentViewAngleCos;

	float cohesionRadius = 1;
	float cohesionWeight = 1;

	float avoidCollisionRadius = 5;
	float avoidCollisionWeight = 5;

	// --- performance
	int alternativeUpdate = 1;
	int alternativeUpdateIndex = 0;



	Vec3 _objScale = Vec3{ T(0.5), T(0.5), T(0.5) };

	// animitation;
	float	_time_scale  = T(0.6);
	float	_speed		 = 1;
	Vec3	_frequency	 = {T(1.0), T(1.3), T(1.2)};
	Vec3	_amplitude	 = {T(0.1), T(0.1), T(0.2)};
	float	_headLimit	 = T(1.0);

};

template<> const TypeInfo* TypeOf<BoidsSetting>();

template<> inline
const TypeInfo* TypeOf<BoidsSetting>() {
	using This = BoidsSetting;
	using TI_Base = TypeInfoInitNoBase<This>;

	class TI : public TI_Base {
	public:
		TI()
			: TI_Base("BoidsSetting")
		{
			static FieldInfo fi[] = {
				{"objectCount",	&This::objectCount },
				{"batchSize",	&This::batchSize },

				{"isUseCuboid " ,			&This::isUseCuboid    },
				{"isDrawCells"  ,			&This::isDrawCells    },
				{"isDrawObstacle"  ,		&This::isDrawObstacle    },
				{"isDrawObjBoundingBox"  ,	&This::isDrawObjBoundingBox    },				
				{"isDrawRay"  ,				&This::isDrawRay    },
				{"isDrawMesh"  ,			&This::isDrawMesh},

				{"lineLength"  ,			&This::lineLength},
				

				// Settings		 
				{"_objScale",		  &This::_objScale },
				{"minSpeed"			, &This::minSpeed		},
				{"maxSpeed"			, &This::maxSpeed		},
				{"maxSteerForce"	, &This::maxSteerForce	},
				{"changeRate"		, &This::changeRate		},
				{"rndSteering"		, &This::rndSteering	},
				{"rndSpeed"			, &This::rndSpeed		},
				{"targetWeight"		, &This::targetWeight	},

				// cuboid
				{"boidsPos"		, &This::boidsPos		},
				{"boidsSize"	, &This::boidsSize		},
				{"minCellSize"  , &This::minCellSize    },
				{"minBoidsSize" , &This::minBoidsSize   },

				// --- Boids param
				{"separationRadius"		, &This::separationRadius },
				{"seperateWeight"		, &This::seperateWeight },

				{"alignmentRadius"			, &This::alignmentRadius },
				{"alignmentWeight"			, &This::alignmentWeight },
				{"alignmentViewAngle"		, &This::alignmentViewAngle },
				{"alignmentViewAngleCos"	, &This::alignmentViewAngleCos },

				{"cohesionRadius"		, &This::cohesionRadius },
				{"cohesionWeight"		, &This::cohesionWeight },

				{"avoidCollisionRadius"		, &This::avoidCollisionRadius },
				{"avoidCollisionWeight"		, &This::avoidCollisionWeight },

				// --- performance
				{"alternativeUpdate"		, &This::alternativeUpdate},
				//{"alternativeUpdateIndex"	, &This::alternativeUpdateIndex},

				// --- animation
				{"_time_scale",		&This::_time_scale },
				{"_speed",			&This::_speed },
				{"_frequency",		&This::_frequency },
				{"_amplitude",		&This::_amplitude },
				{"_headLimit",		&This::_headLimit },
			};
			setFields(fi);
		}
	};
	static TI ti;
	return &ti;

}



}