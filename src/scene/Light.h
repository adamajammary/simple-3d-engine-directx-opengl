#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_LIGHT_H
#define GE3D_LIGHT_H

/*
	http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
	https://learnopengl.com/Lighting/Light-casters

	Distance 	Constant 	Linear 	Quadratic
	7			1.0 		0.7 	1.8
	13			1.0 		0.35 	0.44
	20			1.0 		0.22 	0.20
	32			1.0 		0.14 	0.07
	50			1.0 		0.09 	0.032
	65			1.0			0.07 	0.017
	100 		1.0			0.045 	0.0075
	160 		1.0			0.027 	0.0028
	200 		1.0			0.022 	0.0019
	325 		1.0			0.014 	0.0007
	600			1.0			0.007 	0.0002
	3250		1.0			0.0014 	0.000007
*/
struct Attenuation
{
	Attenuation(float constant, float linear, float quadratic);
	Attenuation();

	float constant;
	float linear;
	float quadratic;
};

struct Light
{
	Light(const glm::vec3 &pos, const Material &mat, const glm::vec3 &dir = {}, const Attenuation &att = {}, float innerAngle = 0, float outerAngle = 0);
	Light();

	bool        active;
	Attenuation attenuation;
	glm::vec3   direction;
	float       innerAngle;
	float       outerAngle;
	Material    material;
	glm::vec3   position;
};

#endif
