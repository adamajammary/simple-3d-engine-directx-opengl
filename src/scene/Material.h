#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_MATERIAL_H
#define S3DE_MATERIAL_H

class Specular
{
public:
	Specular();

public:
	glm::vec3 intensity;
	float     shininess;

};

class Material
{
public:
	Material();

public:
	glm::vec3 ambient;
	glm::vec4 diffuse;
	Specular  specular;
	wxString  textures[MAX_TEXTURES];

};

#endif
