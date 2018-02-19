#include "Skybox.h"

Skybox::Skybox(const wxString &modelFile, const std::vector<wxString> &textureImageFiles) : Component("Skybox")
{
	RenderEngine::Canvas.Window->SetStatusText("Loading the Skybox ...");

	this->modelFile = modelFile;
	this->type      = COMPONENT_SKYBOX;
	this->Children  = Utils::LoadModelFile(modelFile, this);
	this->isValid   = !this->Children.empty();

	if (this->isValid && (this->Children[0] != nullptr))
	{
		this->Children[0]->Name = "Skybox";

		Texture* texture = new Texture(textureImageFiles, "");
		this->Children[0]->LoadTexture(texture, 0);

		for (int i = 1; i < MAX_TEXTURES; i++)
			this->Children[0]->LoadTexture(Utils::EmptyCubemap, i);

		RenderEngine::Canvas.Window->SetStatusText("Loading the Skybox ... OK");
	} else {
		wxMessageBox("ERROR: Failed to load the Skybox.", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		RenderEngine::Canvas.Window->SetStatusText("Loading the Skybox ... FAIL");
	}
}
