#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_MATERIAL_H
#define GE3D_MATERIAL_H

struct Specular
{
	Specular(const glm::vec3 &intensity, float shininess);
	Specular() {}

	glm::vec3 intensity = { 0.1f, 0.1f, 0.1f };
	float     shininess = 32.0f;
};

struct Material
{
	Material(const glm::vec4 &diffuse, const glm::vec3 &ambient, const Specular &specular = {});
	Material() {}

	glm::vec3 ambient  = { 0.8f, 0.8f, 0.8f };
	glm::vec4 diffuse  = { 0.8f, 0.8f, 0.8f, 1.0f };
	Specular  specular = {};
	wxString  textures[MAX_TEXTURES] = {};
};

#endif
