#include "Material.h"

Specular::Specular(const glm::vec3 &intensity, float shininess)
{
	this->intensity = intensity;
	this->shininess = shininess;
}

Material::Material(const glm::vec4 &diffuse, const glm::vec3 &ambient, const Specular &specular)
{
	this->ambient  = ambient;
	this->diffuse  = diffuse;
	this->specular = specular;
}
