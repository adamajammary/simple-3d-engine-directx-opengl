#include "HUD.h"

HUD::HUD(const wxString &modelFile) : Component("HUD")
{
	RenderEngine::Canvas.Window->SetStatusText("Loading the HUD ...");

	this->modelFile   = modelFile;
	this->TextAlign   = Utils::ALIGNMENTS[4];
	this->TextColor   = *wxBLACK;
	this->TextFont    = Utils::FONTS[0];
	this->TextSize    = 20;
	this->Transparent = false;
	this->type        = COMPONENT_HUD;

	this->Children = Utils::LoadModelFile(modelFile, this);
	this->isValid  = !this->Children.empty();

	if (this->isValid && (this->Children[0] != nullptr))
	{
		glm::vec3 position = { 0.72f, 0.7f,  0.0f };
		glm::vec3 scale    = { 0.25f, 0.25f, 0.0f };

		// Invert Y-axis for both mesh and vertex positions on Vulkan
		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN) {
			position.y *= -1;
			scale.y    *= -1;
		}

		this->Children[0]->Name = "HUD";
		this->Children[0]->MoveTo(position);
		this->Children[0]->ScaleTo(scale);

		this->Update("HUD");

		RenderEngine::Canvas.Window->SetStatusText("Loading the HUD ... OK");
	} else {
		wxMessageBox("ERROR: Failed to load the HUD.", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		RenderEngine::Canvas.Window->SetStatusText("Loading the HUD ... FAIL");
	}
}

HUD::HUD()
{
	this->text        = "";
	this->TextAlign   = "";
	this->TextFont    = "";
	this->TextSize    = 0;
	this->Transparent = false;
	this->type        = COMPONENT_HUD;
}

wxString HUD::Text()
{
	return this->text;
}

void HUD::Update(const wxString &newText)
{
	this->text = (!newText.empty() ? newText : this->text);

	if (this->Children[0] != nullptr)
	{
		glm::vec3  scale = this->Children[0]->Scale();
		wxFont     font(this->TextSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, this->TextFont);
		wxSize     size((RenderEngine::Canvas.Size.GetWidth() * std::abs(scale[0])), (RenderEngine::Canvas.Size.GetHeight() * std::abs(scale[1])));
		wxBitmap   bitmap(size, 32); bitmap.UseAlpha(true);
		wxMemoryDC mdc(bitmap);
		wxGCDC     gcdc(mdc);

		if (this->Transparent)
			gcdc.SetBackground(*wxTRANSPARENT_BRUSH);
		else
			gcdc.SetBackground(wxBrush(Utils::ToWxColour(this->Children[0]->Color)));

		gcdc.Clear();
		
		if (this->Children[0]->IsTextured())
		{
			wxImage* textureImage = Utils::LoadImageFile(this->Children[0]->Textures[0]->ImageFile());
			wxBitmap textureBMP   = wxBitmap(*textureImage);

			if (textureImage->HasAlpha())
				textureBMP.UseAlpha(true);

			gcdc.GetGraphicsContext()->DrawBitmap(textureBMP, 0.0, 0.0, size.GetWidth(), size.GetHeight());
			textureImage->Destroy();
		}

		wxString    verticalAlign      = this->TextAlign.substr(0, this->TextAlign.find("-"));
		wxString    horizontalAlign    = this->TextAlign.substr(this->TextAlign.find("-") + 1);
		wxAlignment verticalPosition   = wxALIGN_LEFT;
		wxAlignment horizontalPosition = wxALIGN_TOP;

		if (verticalAlign == "Top")
			verticalPosition = wxALIGN_TOP;
		else if (verticalAlign == "Middle")
			verticalPosition = wxALIGN_CENTER_VERTICAL;
		else if (verticalAlign == "Bottom")
			verticalPosition = wxALIGN_BOTTOM;

		if (horizontalAlign == "Left")
			horizontalPosition = wxALIGN_LEFT;
		else if (horizontalAlign == "Center")
			horizontalPosition = wxALIGN_CENTER_HORIZONTAL;
		else if (horizontalAlign == "Right")
			horizontalPosition = wxALIGN_RIGHT;

		gcdc.SetFont(font);
		gcdc.SetTextForeground(this->TextColor);
		gcdc.DrawLabel(this->text, wxRect(0, 0, size.GetWidth(), size.GetHeight()), (verticalPosition | horizontalPosition));

		wxImage image = bitmap.ConvertToImage();

		if (image.IsOk()) {
			Texture* texture = new Texture(&image, false, true);
			this->Children[0]->LoadTexture(texture, 5);
		}

		image.Destroy();
	}
}
