#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_NOISE_H
#define S3DE_NOISE_H

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
