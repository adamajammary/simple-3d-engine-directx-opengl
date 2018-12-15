#include "Light.h"

Attenuation::Attenuation(float constant, float linear, float quadratic)
{
	this->constant  = constant;
	this->linear    = linear;
	this->quadratic = quadratic;
}

Attenuation::Attenuation()
{
	this->constant  = 0;
	this->linear    = 0;
	this->quadratic = 0;
}

Light::Light(const glm::vec3 &pos, const Material &mat, const glm::vec3 &dir, const Attenuation &att, float innerAngle, float outerAngle)
{
	this->active      = true;
	this->attenuation = att;
	this->direction   = dir;
	this->innerAngle  = innerAngle;
	this->outerAngle  = outerAngle;
	this->material    = mat;
	this->position    = pos;
}

Light::Light()
{
	this->active      = true;
	this->attenuation = {};
	this->direction   = {};
	this->innerAngle  = 0;
	this->outerAngle  = 0;
	this->material    = {};
	this->position    = {};
}
