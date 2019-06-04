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

	int FBO_TEXTURE_HEIGHT = static_cast<int>(
		static_cast<float>(FBO_TEXTURE_SIZE) * RenderEngine::Canvas.AspectRatio
	);

	// WATER REFLECTION - ABOVE WATER
	//this->reflectionFBO = new FrameBuffer(wxSize(320, (int)(320.0f * RenderEngine::Canvas.AspectRatio)), FBO_COLOR, TEXTURE_2D);
	this->reflectionFBO = new FrameBuffer(wxSize(FBO_TEXTURE_SIZE, FBO_TEXTURE_HEIGHT), FBO_COLOR, TEXTURE_2D);

    // WATER REFRACTION - BELOW WATER
	this->refractionFBO = new FrameBuffer(wxSize(FBO_TEXTURE_SIZE, FBO_TEXTURE_HEIGHT), FBO_COLOR, TEXTURE_2D);

    // TEXTURES
	this->Textures[0] = this->reflectionFBO->GetTexture();
	this->Textures[1] = this->refractionFBO->GetTexture();
	this->Textures[2] = new Texture(textureImageFiles[0], false, true);
	this->Textures[3] = new Texture(textureImageFiles[1], false, true);

	for (int i = 4; i < MAX_TEXTURES; i++)
		this->Textures[i] = SceneManager::EmptyTexture;
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
