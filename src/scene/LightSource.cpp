#include "LightSource.h"

LightSource::LightSource(const wxString &modelFile, LightType type) : Component("Light Source")
{
	RenderEngine::Canvas.Window->SetStatusText("Loading the light source ...");

	this->modelFile  = modelFile;
	this->sourceType = type;
	this->type       = COMPONENT_LIGHTSOURCE;
	this->light      = this->initLight();

	for (uint32_t i = 0; i < MAX_TEXTURES; i++)
		this->views[i] = glm::mat4();

	this->updateProjection();
	this->updateView();

	this->Children = Utils::LoadModelFile(modelFile, this);
	this->isValid  = !this->Children.empty();

	if (this->isValid && (this->Children[0] != nullptr))
	{
		switch (this->sourceType) {
			case LIGHT_TYPE_DIRECTIONAL: this->Children[0]->Name = "Directional Light"; break;
			case LIGHT_TYPE_POINT:       this->Children[0]->Name = "Point Light";       break;
			case LIGHT_TYPE_SPOT:        this->Children[0]->Name = "Spot Light";        break;
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
	this->light      = {};
	this->sourceType = LIGHT_TYPE_UNKNOWN;
	this->type       = COMPONENT_LIGHTSOURCE;
}

Light LightSource::initLight()
{
	Light light = {};

	switch (this->sourceType) {
	case LIGHT_TYPE_DIRECTIONAL:
		light.direction        = { 0.5f, -1.0f, -0.2f };
		light.position         = { -2.0f, 3.0f, 3.0f };
		light.material.diffuse = { 0.4f, 0.4f, 0.4f, 1.0f };
		break;
	case LIGHT_TYPE_POINT:
		light.attenuation      = { 1.0f, 0.09f, 0.032f };
		light.position         = { 0.0f, 5.0f, 0.0f };
		light.material.diffuse = { 1.0f, 1.0f, 0.0f, 1.0f };
		break;
	case LIGHT_TYPE_SPOT:
		light.attenuation      = { 1.0f, 0.09f, 0.032f };
		light.direction        = { 0.0f, -1.0f, 0.0f };
		light.position         = { 0.0f, 5.0f,  0.0f };
		light.innerAngle       = glm::radians(12.5f);
		light.outerAngle       = glm::radians(17.5f);
		light.material.diffuse = { 1.0f, 1.0f, 0.0f, 1.0f };
		break;
	default:
		throw;
	}

	light.material.ambient            = { 0.2f, 0.2f, 0.2f };
	light.material.specular.intensity = { 0.6f, 0.6f, 0.6f };
	light.material.specular.shininess = 20.0f;

	return light;
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

glm::mat4 LightSource::MVP(const glm::mat4 &model)
{
	return (this->projection * this->views[0] * model);
}

glm::mat4 LightSource::Projection()
{
	return this->projection;
}

void LightSource::SetActive(bool active)
{
	this->light.active = active;

	if (!this->Children.empty())
		this->Children[0]->ComponentMaterial.diffuse = (!active ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : this->light.material.diffuse);
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

	if (!this->Children.empty())
		this->Children[0]->ComponentMaterial.specular.intensity = intensity;
}

void LightSource::SetSpecularShininess(float shininess)
{
	this->light.material.specular.shininess = shininess;

	if (!this->Children.empty())
		this->Children[0]->ComponentMaterial.specular.shininess = shininess;
}

LightType LightSource::SourceType()
{
	return this->sourceType;
}

void LightSource::updateProjection()
{
	const float ORTHO_SIZE = 30.0f;

	switch (this->sourceType) {
	case LIGHT_TYPE_DIRECTIONAL:
		this->projection = glm::ortho(-ORTHO_SIZE, ORTHO_SIZE, -ORTHO_SIZE, ORTHO_SIZE, 1.0f, ORTHO_SIZE);
		break;
	case LIGHT_TYPE_POINT:
		this->projection = glm::perspective(PI_HALF, 1.0f, 1.0f, 25.0f);
		break;
	case LIGHT_TYPE_SPOT:
		this->projection = glm::perspective(std::acos(this->light.outerAngle), 1.0f, 1.0f, 20.0f);
		break;
	default:
		throw;
	}
}

void LightSource::updateView()
{
	glm::vec3 dir = this->light.direction;
	glm::vec3 pos = this->light.position;

	switch (this->sourceType) {
	case LIGHT_TYPE_DIRECTIONAL:
		this->views[0] = glm::lookAt(pos, (pos + dir), RenderEngine::CameraMain->Up());
		break;
	case LIGHT_TYPE_POINT:
		// 6 view directions: right, left, top, bottom, near, far
		this->views[0] = glm::lookAt(pos, (pos + glm::vec3(1.0,  0.0, 0.0)), glm::vec3(0.0, -1.0, 0.0));
		this->views[1] = glm::lookAt(pos, (pos + glm::vec3(-1.0, 0.0, 0.0)), glm::vec3(0.0, -1.0, 0.0));
		this->views[2] = glm::lookAt(pos, (pos + glm::vec3(0.0,  1.0, 0.0)), glm::vec3(0.0,  0.0, 1.0));
		this->views[3] = glm::lookAt(pos, (pos + glm::vec3(0.0, -1.0, 0.0)), glm::vec3(0.0, 0.0, -1.0));
		this->views[4] = glm::lookAt(pos, (pos + glm::vec3(0.0,  0.0, 1.0)), glm::vec3(0.0, -1.0, 0.0));
		this->views[5] = glm::lookAt(pos, (pos + glm::vec3(0.0, 0.0, -1.0)), glm::vec3(0.0, -1.0, 0.0));
		break;
	case LIGHT_TYPE_SPOT:
		this->views[0] = glm::lookAt(pos, dir, RenderEngine::CameraMain->Up());
		break;
	default:
		throw;
	}
}

glm::mat4 LightSource::View(int index)
{
	return (index < MAX_TEXTURES ? this->views[index] : glm::mat4());
}
