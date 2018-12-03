#include "Light.h"

Light::Light(const glm::vec3 &position, const Material &material)
{
	this->material = material;
	this->position = position;
}

DirectionalLight::DirectionalLight(const glm::vec3 &direction, const Light &light) : Light(light)
{
	this->direction = direction;
}
