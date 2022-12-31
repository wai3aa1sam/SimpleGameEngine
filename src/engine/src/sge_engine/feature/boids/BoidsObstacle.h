#pragma once

namespace sge {

class BoidsObstacle
{
public:


	void createBoundingBox(Vec3f pts[8])
	{
		for (int i = 0; i < 8; i++)
		{
			_bbox.encapsulate(pts[i]);
		}
	}
public:
	//Vec3f  _position;
	BBox3f _bbox;
};

template<class T>
class Singleton
{
public:
	Singleton(T* p)
	{
		SGE_ASSERT(!_instance);
		_instance = p;
	}
	~Singleton()
	{
		SGE_ASSERT(_instance);
		if (_instance)
		{
			//delete _instance;
			_instance = nullptr;
		}
	}

	static T* instance() { return _instance; }
protected:
	static T* _instance;
};

class BoidsObstacleManager : public Singleton<BoidsObstacleManager>
{
	using Base = Singleton<BoidsObstacleManager>;
public:
	BoidsObstacleManager()
		: Base(this)
	{

	}
	
	Vector<BoidsObstacle*> _obstacles;
};

}