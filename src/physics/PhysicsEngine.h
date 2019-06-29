#ifndef S3DE_GLOBALS_H
#include "../globals.h"
#endif

#ifndef S3DE_PHYSICSENGINE_H
#define S3DE_PHYSICSENGINE_H

class PhysicsEngine
{
private:
	PhysicsEngine()  {}
	~PhysicsEngine() {}

public:
	static void CheckRayCasts(const wxMouseEvent &event);
	static void Update();

};

#endif
