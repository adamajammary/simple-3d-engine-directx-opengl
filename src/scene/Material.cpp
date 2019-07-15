#include "Material.h"

Specular::Specular()
{
	this->intensity = { 0.6f, 0.6f, 0.6f };
	this->shininess = 0.0f;
}

Material::Material()
{
	this->ambient  = { 0.6f, 0.6f, 0.6f };
	this->diffuse  = { 0.6f, 0.6f, 0.6f, 1.0f };
	this->specular = {};

	for (uint32_t i = 0; i < MAX_TEXTURES; i++)
		this->textures[i] = "";
}
