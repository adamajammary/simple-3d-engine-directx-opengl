#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_LIGHTSOURCE_H
#define S3DE_LIGHTSOURCE_H

class LightSource : public Component
{
public:
	LightSource(const wxString &modelFile, LightType type);
	LightSource();
	~LightSource() {}

private:
	Light     light;
	glm::mat4 projection;
	LightType sourceType;
	glm::mat4 views[MAX_TEXTURES];

public:
	bool        Active();
	float       ConeInnerAngle();
	float       ConeOuterAngle();
	glm::vec3   Direction();
	Attenuation GetAttenuation();
	Light       GetLight();
	Material    GetMaterial();
	void        MoveBy(const glm::vec3 &amount)      override;
	void        MoveTo(const glm::vec3 &newPosition) override;
	glm::mat4   MVP(const glm::mat4 &model);
	glm::mat4   Projection();
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
	LightType   SourceType();
	void        updateProjection();
	void        updateView();
	glm::mat4   View(int index);

private:
	Light initLight();

};

#endif
