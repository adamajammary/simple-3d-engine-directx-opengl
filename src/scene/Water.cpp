#include "Water.h"

Water::Water(const wxString &modelFile, const std::vector<wxString> &textureImageFiles) : Component("Water")
{
	RenderEngine::Canvas.Window->SetStatusText("Loading Water ...");

	this->modelFile = modelFile;
	this->type      = COMPONENT_WATER;
	this->fbo       = new WaterFBO(textureImageFiles);
	this->Children  = Utils::LoadModelFile(modelFile, this);
	this->isValid   = !this->Children.empty();

	if (this->isValid && (this->Children[0] != nullptr))
	{
		this->Children[0]->Name = "Water";

		for (int i = 0; i < MAX_TEXTURES; i++)
			this->Children[0]->LoadTexture(this->fbo->Textures[i], i);

		RenderEngine::Canvas.Window->SetStatusText("Loading Water ... OK");
	} else {
		wxMessageBox("ERROR: Failed to load Water.", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		RenderEngine::Canvas.Window->SetStatusText("Loading Water ... FAIL");
	}
}

Water::Water()
{
	this->fbo  = nullptr;
	this->type = COMPONENT_WATER;
}

Water::~Water()
{
	_DELETEP(this->fbo);
}

WaterFBO* Water::FBO()
{
    return this->fbo;
}
