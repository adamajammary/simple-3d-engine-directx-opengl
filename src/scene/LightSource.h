#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_LIGHTSOURCE_H
#define GE3D_LIGHTSOURCE_H

class LightSource : public Component
{
public:
	LightSource(const wxString &modelFile, IconType type);
	LightSource();
	~LightSource() {}

private:
	glm::vec4 color;
	IconType  sourceType;
	Light     light;

public:
	bool        Active();
	glm::vec3   Direction();
	Attenuation GetAttenuation();
	float       GetConeInnerAngle();
	float       GetConeOuterAngle();
	Material    GetMaterial();
	Light       GetLight();
	void        MoveBy(const glm::vec3 &amount)      override;
	void        MoveTo(const glm::vec3 &newPosition) override;
	void        SetActive(bool active);
	void        SetAmbient(const glm::vec3 &ambient);
	void        SetAttenuationLinear(float linear);
	void        SetAttenuationQuadratic(float quadratic);
	void        SetColor(const glm::vec4 &color);
	void        SetConeInnerAngle(float angleRad);
	void        SetConeOuterAngle(float angleRad);
	void        SetDirection(const glm::vec3 &direction);
	void        SetSpecularIntensity(const glm::vec3 &intensity);
	void        SetSpecularShininess(float shininess);
	IconType    SourceType();

};

#endif
