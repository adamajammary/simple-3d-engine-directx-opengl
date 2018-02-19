#include "Model.h"

Model::Model(const wxString &modelFile) : Component("Model")
{
	RenderEngine::Canvas.Window->SetStatusText("Loading '" + modelFile + "' ...");

	this->modelFile = modelFile;
	this->type      = COMPONENT_MODEL;
	this->Children  = Utils::LoadModelFile(modelFile, this);
	this->isValid   = !this->Children.empty();

	if (this->isValid && (this->Children[0] != nullptr))
		RenderEngine::Canvas.Window->SetStatusText("Loading '" + modelFile + "' ... OK");
	else
		RenderEngine::Canvas.Window->SetStatusText("Loading '" + modelFile + "' ... FAIL");
}
