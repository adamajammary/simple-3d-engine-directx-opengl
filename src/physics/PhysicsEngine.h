#ifndef GE3D_GLOBALS_H
#include "../globals.h"
#endif

#ifndef GE3D_PHYSICSENGINE_H
#define GE3D_PHYSICSENGINE_H

class PhysicsEngine
{
private:
	PhysicsEngine()  {}
	~PhysicsEngine() {}

public:
	static void CheckRayCasts(wxMouseEvent &event);
	static void Update();

};

#endif
