#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_NOISE_H
#define GE3D_NOISE_H

class Noise
{
private:
	Noise()  {}
	~Noise() {}

private:
	static noise::module::Perlin simplex;

public:
	static float Height(int x, int z, int octaves, float redistribution);

};

#endif
