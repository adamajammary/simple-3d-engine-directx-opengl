#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_LIGHT_H
#define GE3D_LIGHT_H

struct Light
{
	Light(const glm::vec3 &position, const Material &material);
	Light() {}

	Material  material = {};
	glm::vec3 position = {};
};

struct DirectionalLight : Light
{
	DirectionalLight(const glm::vec3 &direction, const Light &light);
	DirectionalLight() {}

	glm::vec3 direction = {};
};

#endif
