#include "Noise.h"

noise::module::Perlin Noise::simplex;

float Noise::Height(int x, int z, int octaves, float redistribution)
{
	float height    = 0.0f;
	float frequency = 1.0f;
	float octave    = 1.0f;
	float octaveSum = 0.0f;

	for (int i = 0; i < octaves; i++)
	{
		height    += (octave * Noise::simplex.GetValue((frequency * (float)x), (frequency * (float)z * 0.5f + 0.5f), 0.0f));
		octaveSum += octave;
		frequency *= 2.0f;
		octave    *= 0.5f;
	}

	height /= octaveSum;
	height  = std::pow(height, redistribution);

	return height;
}
