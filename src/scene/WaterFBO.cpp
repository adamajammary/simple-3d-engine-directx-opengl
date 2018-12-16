#include "WaterFBO.h"

WaterFBO::WaterFBO(const std::vector<wxString> &textureImageFiles)
{
	this->moveFactor    = 0.0f;
	this->reflectionFBO = nullptr;
	this->refractionFBO = nullptr;
	this->Speed         = 0.05f;
	this->WaveStrength  = 0.05f;

	for (int i = 0; i < MAX_TEXTURES; i++)
		this->Textures[i] = nullptr;

	// WATER REFLECTION - ABOVE WATER
	this->reflectionFBO = new FrameBuffer(320, (int)(320.0f * RenderEngine::Canvas.AspectRatio));

	if (this->reflectionFBO != nullptr)
		this->reflectionFBO->CreateColorTexture();

    // WATER REFRACTION - BELOW WATER
	this->refractionFBO = new FrameBuffer(1280, (int)(1280.0f * RenderEngine::Canvas.AspectRatio));

	if (this->refractionFBO != nullptr)
		this->refractionFBO->CreateColorTexture();

    // TEXTURES
	if ((this->reflectionFBO != nullptr) && (this->refractionFBO != nullptr))
	{
		this->Textures[0] = this->reflectionFBO->ColorTexture();
		this->Textures[1] = this->refractionFBO->ColorTexture();
		this->Textures[2] = new Texture(textureImageFiles[0], false, true);
		this->Textures[3] = new Texture(textureImageFiles[1], false, true);

		for (int i = 4; i < MAX_TEXTURES; i++)
			this->Textures[i] = SceneManager::EmptyTexture;
	} else {
		for (int i = 0; i < MAX_TEXTURES; i++)
			this->Textures[i] = SceneManager::EmptyTexture;
	}
}

WaterFBO::WaterFBO()
{
	this->moveFactor    = 0.0f;
	this->reflectionFBO = nullptr;
	this->refractionFBO = nullptr;
	this->Speed         = 0.0f;
	this->WaveStrength  = 0.0f;

	for (int i = 0; i < MAX_TEXTURES; i++)
		this->Textures[i] = nullptr;
}

WaterFBO::~WaterFBO()
{
	this->moveFactor   = 0.0f;
	this->Speed        = 0.0f;
	this->WaveStrength = 0.0f;

	if ((this->reflectionFBO != nullptr) && (this->refractionFBO != nullptr))
	{
		_DELETEP(this->reflectionFBO);
		_DELETEP(this->refractionFBO);
		_DELETEP(this->Textures[2]);
		_DELETEP(this->Textures[3]);
	}

	for (int i = 0; i < MAX_TEXTURES; i++)
		this->Textures[i] = nullptr;
}

void WaterFBO::BindReflection()
{
	if (this->reflectionFBO != nullptr)
		this->reflectionFBO->Bind();
}

void WaterFBO::BindRefraction()
{
	if (this->refractionFBO != nullptr)
		this->refractionFBO->Bind();
}

float WaterFBO::MoveFactor()
{
	this->moveFactor += (this->Speed * TimeManager::DeltaTime);
	this->moveFactor  = (this->moveFactor >= 1.0f ? 0.0f : this->moveFactor);

	return this->moveFactor;
}

FrameBuffer* WaterFBO::ReflectionFBO()
{
	return this->reflectionFBO;
}

FrameBuffer* WaterFBO::RefractionFBO()
{
	return this->refractionFBO;
}

void WaterFBO::UnbindReflection()
{
	if (this->reflectionFBO != nullptr)
		this->reflectionFBO->Unbind();
}

void WaterFBO::UnbindRefraction()
{
	if (this->refractionFBO != nullptr)
		this->refractionFBO->Unbind();
}
