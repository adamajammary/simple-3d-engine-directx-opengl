#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_TIMEMANAGER_H
#define S3DE_TIMEMANAGER_H

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
	static long TimeElapsedMS();
	static void UpdateFPS();

};

#endif
