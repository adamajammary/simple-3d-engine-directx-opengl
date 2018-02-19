#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_TIMEMANAGER_H
#define GE3D_TIMEMANAGER_H

class TimeManager
{
private:
	TimeManager()  {}
	~TimeManager() {}

public:
	static double DeltaTime;
	static int    FPS;

private:
	static wxStopWatch deltaTimer;
	static wxStopWatch totalTimer;

public:
	static void Start();
	static void UpdateFPS();

};

#endif
