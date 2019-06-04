#include "LightSource.h"

LightSource::LightSource(const wxString &modelFile, IconType type) : Component("Light Source")
{
	RenderEngine::Canvas.Window->SetStatusText("Loading the light source ...");

	this->modelFile   = modelFile;
	this->sourceType  = type;
	this->type        = COMPONENT_LIGHTSOURCE;
	this->depthMapFBO = this->initDepthMap();
	this->light       = this->initLight();

	this->updateProjection();
	this->updateView();

	this->Children = Utils::LoadModelFile(modelFile, this);
	this->isValid  = !this->Children.empty();

	if (this->isValid && !this->Children.empty())
	{
		switch (type) {
			case ID_ICON_LIGHT_DIRECTIONAL: this->Children[0]->Name = "Directional Light"; break;
			case ID_ICON_LIGHT_POINT:       this->Children[0]->Name = "Point Light";       break;
			case ID_ICON_LIGHT_SPOT:        this->Children[0]->Name = "Spot Light";        break;
			default: throw;
		}

		this->Children[0]->ComponentMaterial = this->light.material;
		this->Children[0]->MoveTo(this->light.position);

		this->Children[0]->ScaleTo({ 0.25f, 0.25f, 0.25f });

		RenderEngine::Canvas.Window->SetStatusText("Loading the light source ... OK");
	} else {
		wxMessageBox("ERROR: Failed to load the light source.", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		RenderEngine::Canvas.Window->SetStatusText("Loading the light source ... FAIL");
	}
}

LightSource::LightSource()
{
	this->depthMapFBO = nullptr;
	this->light       = {};
	this->sourceType  = ID_ICON_UNKNOWN;
	this->type        = COMPONENT_LIGHTSOURCE;
}

FrameBuffer* LightSource::initDepthMap()
{
	_DELETEP(this->depthMapFBO);

	switch (this->sourceType) {
	case ID_ICON_LIGHT_DIRECTIONAL:
		this->depthMapFBO = new FrameBuffer(wxSize(FBO_TEXTURE_SIZE, FBO_TEXTURE_SIZE), FBO_DEPTH, TEXTURE_2D);
		break;
	case ID_ICON_LIGHT_POINT:
		//this->depthMapFBO = new FrameBuffer(wxSize(FBO_TEXTURE_SIZE, FBO_TEXTURE_SIZE), TEXTURE_CUBEMAP);
		break;
	case ID_ICON_LIGHT_SPOT:
		//this->depthMapFBO = new FrameBuffer(wxSize(FBO_TEXTURE_SIZE, FBO_TEXTURE_SIZE), TEXTURE_CUBEMAP);
		break;
	default:
		throw;
	}

	return this->depthMapFBO;
}

Light LightSource::initLight()
{
	this->light = {};

	switch (this->sourceType) {
	case ID_ICON_LIGHT_DIRECTIONAL:
		//this->light.direction        = { 0.2f, -1.0f, -0.3f };
		//this->light.position         = { -2.0f, 4.0f, -1.0f };
		this->light.direction        = { 0.5f, -1.0f, -0.2f };
		this->light.position         = { -2.0f, 3.0f, 3.0f };
		this->light.material.diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };

		break;
	case ID_ICON_LIGHT_POINT:
		this->light.attenuation      = { 1.0f, 0.09f, 0.032f };
		this->light.position         = { 0.0f, 5.0f, 0.0f };
		this->light.material.diffuse = { 1.0f, 1.0f, 0.0f, 1.0f };

		break;
	case ID_ICON_LIGHT_SPOT:
		this->light.attenuation      = { 1.0f, 0.09f, 0.032f };
		this->light.direction        = { 0.0f, -1.0f, 0.0f };
		this->light.position         = { 0.0f, 5.0f,  0.0f };
		this->light.innerAngle       = glm::radians(12.5f);
		this->light.outerAngle       = glm::radians(17.5f);
		this->light.material.diffuse = { 1.0f, 1.0f, 0.0f, 1.0f };

		break;
	default:
		throw;
	}

	this->light.material.ambient            = { 0.2f, 0.2f, 0.2f };
	this->light.material.specular.intensity = { 0.6f, 0.6f, 0.6f };
	this->light.material.specular.shininess = 20.0f;

	this->color = this->light.material.diffuse;

	return this->light;
}

LightSource::~LightSource()
{
	_DELETEP(this->depthMapFBO);
}

bool LightSource::Active()
{
	return this->light.active;
}

float LightSource::ConeInnerAngle()
{
	return this->light.innerAngle;
}

float LightSource::ConeOuterAngle()
{
	return this->light.outerAngle;
}

FrameBuffer* LightSource::DepthMapFBO()
{
	return this->depthMapFBO;
}

glm::vec3 LightSource::Direction()
{
	return this->light.direction;
}

Attenuation LightSource::GetAttenuation()
{
	return this->light.attenuation;
}

Light LightSource::GetLight()
{
	return this->light;
}

Material LightSource::GetMaterial()
{
	return this->light.material;
}

void LightSource::MoveBy(const glm::vec3 &amount)
{
	this->light.position += amount;

	if (!this->Children.empty())
		this->Children[0]->MoveBy(amount);

	this->updateView();
}

void LightSource::MoveTo(const glm::vec3 &newPosition)
{
	this->light.position = newPosition;

	if (!this->Children.empty())
		this->Children[0]->MoveTo(newPosition);

	this->updateView();
}

glm::mat4 LightSource::MVP(Component* model)
{
	return (this->projection * this->view * model->Matrix());
}

glm::mat4 LightSource::Projection()
{
	return this->projection;
}

void LightSource::SetActive(bool active)
{
	this->light.active = active;

	if (!this->Children.empty())
		this->Children[0]->ComponentMaterial.diffuse = (!active ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : this->color);
}

void LightSource::SetAmbient(const glm::vec3 &ambient)
{
	this->light.material.ambient = ambient;
}

void LightSource::SetAttenuationLinear(float linear)
{
	this->light.attenuation.linear = linear;
}

void LightSource::SetAttenuationQuadratic(float quadratic)
{
	this->light.attenuation.quadratic = quadratic;
}

void LightSource::SetColor(const glm::vec4 &color)
{
	this->light.material.diffuse = color;
	this->color                  = color;

	if (!this->Children.empty())
		this->Children[0]->ComponentMaterial.diffuse = color;
}

void LightSource::SetConeInnerAngle(float angleRad)
{
	this->light.innerAngle = angleRad;
}

void LightSource::SetConeOuterAngle(float angleRad)
{
	this->light.outerAngle = angleRad;
}

void LightSource::SetDirection(const glm::vec3 &direction)
{
	this->light.direction = direction;

	this->updateView();
}

void LightSource::SetSpecularIntensity(const glm::vec3 &intensity)
{
	this->light.material.specular.intensity = intensity;
}

void LightSource::SetSpecularShininess(float shininess)
{
	this->light.material.specular.shininess = shininess;
}

IconType LightSource::SourceType()
{
	return this->sourceType;
}

void LightSource::updateProjection()
{
	const float PROJECTION_SIZE = 30.0f;

	this->projection = glm::ortho(
		-PROJECTION_SIZE, PROJECTION_SIZE, -PROJECTION_SIZE, PROJECTION_SIZE,
		1.0f, PROJECTION_SIZE
	);
}

void LightSource::updateView()
{
	glm::vec3 lightPosition  = { this->light.position.x,  this->light.position.y,  this->light.position.z  };
	glm::vec3 lightDirection = { this->light.direction.x, this->light.direction.y, this->light.direction.z };

	//this->view = glm::lookAt(lightPosition, glm::vec3(0.0f), RenderEngine::CameraMain->Up());
	this->view = glm::lookAt(lightPosition, (lightPosition + lightDirection), RenderEngine::CameraMain->Up());
}

glm::mat4 LightSource::View()
{
	return this->view;
}
