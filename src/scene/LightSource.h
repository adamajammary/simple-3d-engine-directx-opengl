#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_LIGHTSOURCE_H
#define S3DE_LIGHTSOURCE_H

class LightSource : public Component
{
public:
	LightSource(const wxString &modelFile, IconType type);
	LightSource();
	~LightSource();

private:
	glm::vec4    color;
	FrameBuffer* depthMapFBO;
	Light        light;
	glm::mat4    projection;
	IconType     sourceType;
	glm::mat4    view;

public:
	bool         Active();
	float        ConeInnerAngle();
	float        ConeOuterAngle();
	FrameBuffer* DepthMapFBO();
	glm::vec3    Direction();
	Attenuation  GetAttenuation();
	Light        GetLight();
	Material     GetMaterial();
	void         MoveBy(const glm::vec3 &amount)      override;
	void         MoveTo(const glm::vec3 &newPosition) override;
	glm::mat4    MVP(Component* model);
	glm::mat4    Projection();
	void         SetActive(bool active);
	void         SetAmbient(const glm::vec3 &ambient);
	void         SetAttenuationLinear(float linear);
	void         SetAttenuationQuadratic(float quadratic);
	void         SetColor(const glm::vec4 &color);
	void         SetConeInnerAngle(float angleRad);
	void         SetConeOuterAngle(float angleRad);
	void         SetDirection(const glm::vec3 &direction);
	void         SetSpecularIntensity(const glm::vec3 &intensity);
	void         SetSpecularShininess(float shininess);
	IconType     SourceType();
	void         updateProjection();
	void         updateView();
	glm::mat4    View();

private:
	FrameBuffer* initDepthMap();
	Light        initLight();

};

#endif
