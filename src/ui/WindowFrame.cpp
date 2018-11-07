#include "WindowFrame.h"

WindowFrame::WindowFrame(const wxString &title, const wxPoint &pos, const wxSize &size, Window* parent) : wxFrame(NULL, wxID_ANY, title, pos, size)
{
	this->Parent = parent;

	// ICON
	wxIcon icon = wxIcon("img/icon_64.ico", wxBITMAP_TYPE_ICO);
	this->SetIcon(icon);

	// MENU
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	this->SetMenuBar(menuBar);

	// INIT
	this->SetForegroundColour(wxColour(0xFF, 0xFF, 0xFF, 0xFF));
	this->SetBackgroundColour(wxColour(0x20, 0x20, 0x20, 0xFF));
	this->Centre();

	// MAIN
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(sizer);

	// ADS
	#ifndef _DEBUG
		wxStaticBoxSizer* sizerAds = new wxStaticBoxSizer(wxHORIZONTAL, this->Parent->GetTopWindow(), "Google Ads");
		wxWebView*        webView  = nullptr;

		for (int i = 0; i < 2; i++) {
			webView = wxWebView::New(this, wxID_ANY, "http://jammary.com/google_ad_horizontal.html", wxDefaultPosition, wxSize(730, 90), wxWebViewBackendDefault, wxBORDER_NONE);
			sizerAds->Add(webView, 0, wxALL, 10);
		}

		sizer->Add(sizerAds, 0, wxALL, 10);
	#endif

	// TOP - ICONS
	wxBoxSizer* sizerIcons = new wxBoxSizer(wxHORIZONTAL);

	wxButton* button = new wxButton(this, ID_ICON_BROWSE, "BROWSE", wxDefaultPosition, wxSize(-1, 45));
	button->Bind(wxEVT_BUTTON, &InputManager::OnIcon, ID_ICON_BROWSE, ID_ICON_BROWSE);
	sizerIcons->Add(button, 0, wxLEFT, 10);

	for (int i = 0; i < (int)Utils::ICONS.size(); i++)
	{
		wxImage image = wxImage(Utils::ICONS[i].File, wxBITMAP_TYPE_PNG);
		image.Rescale(30, 30, wxIMAGE_QUALITY_BICUBIC);

		button = new wxButton(this, Utils::ICONS[i].ID, Utils::ICONS[i].Title);
		button->SetBitmapLabel(image);

		button->SetForegroundColour(wxColour(0xFF, 0xFF, 0xFF, 0xFF));
		button->SetBackgroundColour(wxColour(0x20, 0x20, 0x20, 0xFF));
		button->SetBitmapMargins(wxSize(5, 5));

		button->Bind(wxEVT_BUTTON, &InputManager::OnIcon, Utils::ICONS[i].ID, Utils::ICONS[i].ID);

		sizerIcons->Add(button, 0, wxLEFT, 10);
	}

	sizer->Add(sizerIcons);

	// LINE SEPARATOR
	wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(1, 1), wxLI_HORIZONTAL);
	line->SetForegroundColour(wxColour(0xFF, 0xFF, 0xFF, 0xFF));
	sizer->Add(line, 0, (wxEXPAND | wxLEFT | wxRIGHT | wxTOP), 10);

	// TOP - DROPDOWNS
	wxGridBagSizer* sizerDropDowns = new wxGridBagSizer();

	// GRAPHICS API
	wxStaticText* label = new wxStaticText(this, wxID_ANY, "Graphics API:");
	wxFont        font  = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	sizerDropDowns->Add(label, wxGBPosition(1, 1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

	this->dropDownGraphicsAPIs = new wxChoice(this, ID_GRAPHICS_API, wxDefaultPosition, wxDefaultSize, 4, Utils::GRAPHIC_APIS);
	this->dropDownGraphicsAPIs->Select(0);
	this->dropDownGraphicsAPIs->Bind(wxEVT_CHOICE, &InputManager::OnGraphicsMenu, ID_GRAPHICS_API, ID_GRAPHICS_API);
	sizerDropDowns->Add(this->dropDownGraphicsAPIs, wxGBPosition(1, 2), wxDefaultSpan, wxLEFT, 10);

	// ASPECT RATIO
	label = new wxStaticText(this, wxID_ANY, "Aspect Ratio:");
	font = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	sizerDropDowns->Add(label, wxGBPosition(1, 3), wxDefaultSpan, (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);

	wxChoice* dropDown = new wxChoice(this, ID_ASPECT_RATIO, wxDefaultPosition, wxDefaultSize, 2, Utils::ASPECT_RATIOS);
	dropDown->Select(0);
	dropDown->Bind(wxEVT_CHOICE, &InputManager::OnGraphicsMenu, ID_ASPECT_RATIO, ID_ASPECT_RATIO);
	sizerDropDowns->Add(dropDown, wxGBPosition(1, 4), wxDefaultSpan, wxLEFT, 10);

	// FOV
	label = new wxStaticText(this, wxID_ANY, "FOV:");
	font  = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	sizerDropDowns->Add(label, wxGBPosition(1, 5), wxDefaultSpan, (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);

	dropDown = new wxChoice(this, ID_FOV, wxDefaultPosition, wxDefaultSize, 4, Utils::FOVS);
	dropDown->Select(0);
	dropDown->Bind(wxEVT_CHOICE, &InputManager::OnGraphicsMenu, ID_FOV, ID_FOV);
	sizerDropDowns->Add(dropDown, wxGBPosition(1, 6), wxDefaultSpan, wxLEFT, 10);

	// DRAW MODE
	label = new wxStaticText(this, wxID_ANY, "Draw Mode:");
	font  = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	sizerDropDowns->Add(label, wxGBPosition(1, 7), wxDefaultSpan, (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);

	this->dropDownDrawModes = new wxChoice(this, ID_DRAW_MODE, wxDefaultPosition, wxDefaultSize, 2, Utils::DRAW_MODES);
	this->dropDownDrawModes->Select(0);
	this->dropDownDrawModes->Bind(wxEVT_CHOICE, &InputManager::OnGraphicsMenu, ID_DRAW_MODE, ID_DRAW_MODE);
	sizerDropDowns->Add(this->dropDownDrawModes, wxGBPosition(1, 8), wxDefaultSpan, wxLEFT, 10);

	// DRAW BOUNDING VOLUMES
	label = new wxStaticText(this, wxID_ANY, "Draw Bounding Volumes:");
	font  = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	sizerDropDowns->Add(label, wxGBPosition(1, 9), wxDefaultSpan, (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);

	wxCheckBox* checkBox = new wxCheckBox(this, ID_DRAW_BOUNDING, "");
	checkBox->Bind(wxEVT_CHECKBOX, &InputManager::OnGraphicsMenu, ID_DRAW_BOUNDING, ID_DRAW_BOUNDING);
	sizerDropDowns->Add(checkBox, wxGBPosition(1, 10), wxDefaultSpan, (wxALIGN_CENTER_VERTICAL | wxLEFT), 10);

	// VSYNC
	label = new wxStaticText(this, wxID_ANY, "V-sync:");
	font  = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	sizerDropDowns->Add(label, wxGBPosition(1, 11), wxDefaultSpan, (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);

	this->VSyncEnable = new wxCheckBox(this, ID_VSYNC, "");
	this->VSyncEnable->Bind(wxEVT_CHECKBOX, &InputManager::OnGraphicsMenu, ID_VSYNC, ID_VSYNC);
	this->VSyncEnable->SetValue(true);
	sizerDropDowns->Add(this->VSyncEnable, wxGBPosition(1, 12), wxDefaultSpan, (wxALIGN_CENTER_VERTICAL | wxLEFT), 10);

	sizer->Add(sizerDropDowns, 0, wxBOTTOM, 10);

	// LINE SEPARATOR
	line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(1, 1), wxLI_HORIZONTAL);
	line->SetForegroundColour(wxColour(0xFF, 0xFF, 0xFF, 0xFF));
	sizer->Add(line, 0, (wxEXPAND | wxLEFT | wxRIGHT | wxTOP), 10);

	// MIDDLE
	this->sizerMiddle = new wxBoxSizer(wxHORIZONTAL);

	// MIDDLE-LEFT - CANVAS

	// MIDDLE-RIGHT
	wxBoxSizer* sizerMiddleRight = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* sizerSceneMenu = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(this, wxID_ANY, "Scene");
	font  = label->GetFont(); font.SetWeight(wxFONTWEIGHT_BOLD); label->SetFont(font);
	sizerSceneMenu->Add(label, 0, (wxALIGN_CENTER_VERTICAL | wxLEFT), 10);

	button = new wxButton(this, ID_SCENE_SAVE, "SAVE");
	button->Bind(wxEVT_BUTTON, &InputManager::OnList, ID_SCENE_SAVE, ID_SCENE_SAVE);
	sizerSceneMenu->Add(button, 0, wxLEFT, 10);

	button = new wxButton(this, ID_SCENE_LOAD, "LOAD");
	button->Bind(wxEVT_BUTTON, &InputManager::OnList, ID_SCENE_LOAD, ID_SCENE_LOAD);
	sizerSceneMenu->Add(button, 0, wxLEFT, 10);

	button = new wxButton(this, ID_SCENE_CLEAR, "CLEAR");
	button->Bind(wxEVT_BUTTON, &InputManager::OnList, ID_SCENE_CLEAR, ID_SCENE_CLEAR);
	sizerSceneMenu->Add(button, 0, wxLEFT, 10);

	sizerMiddleRight->Add(sizerSceneMenu);

	// LINE SEPARATOR
	line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(1, 1), wxLI_HORIZONTAL);
	line->SetForegroundColour(wxColour(0xFF, 0xFF, 0xFF, 0xFF));
	sizerMiddleRight->Add(line, 0, (wxEXPAND | wxLEFT | wxTOP | wxBOTTOM), 10);

	wxBoxSizer* sizerSceneComponents = new wxBoxSizer(wxHORIZONTAL);

	// COMPONENTS
	this->listBoxComponents = new wxListBox(this, ID_COMPONENTS, wxDefaultPosition, wxSize(370, 200), 0, 0, (wxLB_SINGLE | wxLB_ALWAYS_SB));
	this->listBoxComponents->Bind(wxEVT_LISTBOX, &InputManager::OnList, ID_COMPONENTS, ID_COMPONENTS);
	sizerSceneComponents->Add(this->listBoxComponents, 0, wxLEFT, 10);

	// CHILDREN
	this->listBoxChildren = new wxListBox(this, ID_CHILDREN, wxDefaultPosition, wxSize(370, 200), 0, 0, (wxLB_SINGLE | wxLB_ALWAYS_SB));
	this->listBoxChildren->Bind(wxEVT_LISTBOX, &InputManager::OnList, ID_CHILDREN, ID_CHILDREN);
	sizerSceneComponents->Add(this->listBoxChildren, 0, wxLEFT, 10);

	sizerMiddleRight->Add(sizerSceneComponents);

	// DETAILS
	this->sizerSceneDetails = new wxBoxSizer(wxVERTICAL);

	sizerMiddleRight->Add(this->sizerSceneDetails, 1, (wxEXPAND | wxLEFT | wxTOP), 10);

	this->sizerMiddle->Add(sizerMiddleRight, 1, wxEXPAND);

	sizer->Add(this->sizerMiddle, 1, (wxEXPAND | wxLEFT | wxTOP), 10);

	// STATUS BAR
	this->CreateStatusBar();
}

void WindowFrame::AddListComponent(Component* component)
{
	this->listBoxComponents->AppendString(component->Name);
	this->SelectComponent(this->listBoxComponents->GetCount() - 1);
}

void WindowFrame::AddListChildren(std::vector<Mesh*> children)
{
	this->listBoxChildren->Clear();

	for (auto child : children) {
		SceneManager::SelectedChild = child;
		this->listBoxChildren->AppendString(child->Name);
	}

	this->listBoxChildren->Select(this->listBoxChildren->GetCount() - 1);
}

void WindowFrame::addPropertyCheckbox(const wxString &label, const wxString &id, bool value)
{
	this->propertyGrid->Append(new wxBoolProperty(label, id, value));
	this->propertyGrid->SetPropertyEditor(id, wxPGEditor_CheckBox);
}

void WindowFrame::addPropertyEnum(const wxString &label, const wxString &id, const wxChar** values, const wxChar* value)
{
	this->propertyGrid->Append(new wxEnumProperty(label, id, values));
	this->propertyGrid->SetPropertyValue(id, value);
}

void WindowFrame::addPropertyRange(const wxString &label, const wxString &id, float value, float min, float max, float step)
{
	this->propertyGrid->Append(new wxFloatProperty(label, id, value));
	this->propertyGrid->SetPropertyEditor(id,    wxPGEditor_SpinCtrl);
	this->propertyGrid->SetPropertyAttribute(id, wxPG_ATTR_MIN, min);
	this->propertyGrid->SetPropertyAttribute(id, wxPG_ATTR_MAX, max);
	this->propertyGrid->SetPropertyAttribute(id, wxPG_ATTR_SPINCTRL_STEP, step);
}

void WindowFrame::addPropertyXYZ(const wxString &category, const wxString &id, float x, float y, float z, float min, float max, float step)
{
	this->propertyGrid->Append(new wxPropertyCategory(category));
	this->addPropertyRange("X", wxString(id + "_X"), x, min, max, step);
	this->addPropertyRange("Y", wxString(id + "_Y"), y, min, max, step);
	this->addPropertyRange("Z", wxString(id + "_Z"), z, min, max, step);
}

void WindowFrame::ClearScene()
{
	this->listBoxComponents->Clear();
	this->listBoxChildren->Clear();

	// Add camera back to the scene
	if (RenderEngine::Camera != NULL) {
		RenderEngine::Camera->UpdateProjection();
		SceneManager::AddComponent(RenderEngine::Camera);
	}
}

void WindowFrame::DeactivateDetails()
{
	this->propertyGrid->UnfocusEditor();
}

void WindowFrame::InitDetails()
{
	this->sizerSceneDetails->Clear(true);

	wxButton*   button;
	wxBoxSizer* sizerButtons = new wxBoxSizer(wxHORIZONTAL);

	// REMOVE COMPONENT
	if ((SceneManager::SelectedComponent != nullptr) && (SceneManager::SelectedComponent->Type() != COMPONENT_CAMERA))
	{
		button = new wxButton(this, ID_REMOVE_COMPONENT, wxString("REMOVE " + SceneManager::SelectedComponent->Name));
		button->Bind(wxEVT_BUTTON, &InputManager::OnList, ID_REMOVE_COMPONENT, ID_REMOVE_COMPONENT);
		sizerButtons->Add(button, 0, wxBOTTOM, 10);
	}

	// REMOVE CHILD
	if (SceneManager::SelectedChild != nullptr)
	{
		button = new wxButton(this, ID_REMOVE_CHILD, wxString("REMOVE " + SceneManager::SelectedChild->Name));
		button->Bind(wxEVT_BUTTON, &InputManager::OnList, ID_REMOVE_CHILD, ID_REMOVE_CHILD);
		sizerButtons->Add(button, 0, wxLEFT, 10);
	}

	this->sizerSceneDetails->Add(sizerButtons);

	this->propertyGrid = new wxPropertyGrid(this, ID_SCENE_DETAILS, wxDefaultPosition, wxDefaultSize, wxPG_SPLITTER_AUTO_CENTER);
	this->propertyGrid->SetEmptySpaceColour(wxColour(0x40, 0x40, 0x40, 0xFF));
	this->sizerSceneDetails->Add(this->propertyGrid, 1, (wxEXPAND | wxRIGHT | wxBOTTOM), 10);
	this->propertyGrid->Bind(wxEVT_PG_CHANGED, &InputManager::OnPropertyChanged, ID_SCENE_DETAILS, ID_SCENE_DETAILS);

	Component* selected = (SceneManager::SelectedChild != nullptr ? SceneManager::SelectedChild : SceneManager::SelectedComponent);

	if ((selected != nullptr) && (this->propertyGrid != nullptr))
	{
		this->propertyGrid->Clear();

		// NAME
		this->propertyGrid->Append(new wxStringProperty("Name", "ID_NAME", static_cast<wxString>(selected->Name)));
		this->propertyGrid->GetProperty("ID_NAME")->SetValidator(wxTextValidator(wxFILTER_EMPTY));

		if (selected->Type() == COMPONENT_CAMERA)
			this->propertyGrid->SetPropertyReadOnly("ID_NAME");

		// LOCATION
		if (selected->Type() != COMPONENT_SKYBOX) {
			glm::vec3 position = selected->Position();
			this->addPropertyXYZ("Location", "ID_LOCATION", position[0], position[1], position[2], -100.0f, 100.0f, 0.01f);
		}

		// ROTATION
		if (selected->Type() != COMPONENT_SKYBOX) {
			glm::vec3 rotation = selected->Rotation();
			this->addPropertyXYZ("Rotation (rad)", "ID_ROTATION", rotation[0], rotation[1], rotation[2], -glm::pi<float>(), glm::pi<float>(), 0.01f);
		}

		// SCALE
		if ((selected->Type() != COMPONENT_CAMERA) && (selected->Type() != COMPONENT_SKYBOX)) {
			glm::vec3 scale = selected->Scale();
			this->addPropertyXYZ("Scale", "ID_SCALE", scale[0], scale[1], scale[2], -100.0f, 100.0f, 0.01f);
		}

		// AUTO-ROTATE
		if ((selected->Type() != COMPONENT_CAMERA) && (selected->Type() != COMPONENT_SKYBOX)) {
			glm::vec3 autoRotation = selected->AutoRotation;
			this->addPropertyXYZ("Auto-Rotate (rad)", "ID_AUTO_ROTATION", autoRotation[0], autoRotation[1], autoRotation[2], -glm::pi<float>(), glm::pi<float>(), 0.01f);
			this->addPropertyCheckbox("Enable", "ID_ENABLE_AUTO_ROTATION", selected->AutoRotate);
		}

		// MATERIAL
		if ((selected->Type() != COMPONENT_CAMERA) && (selected->Type() != COMPONENT_SKYBOX)) {
			this->propertyGrid->Append(new wxPropertyCategory("Material"));
			this->propertyGrid->Append(new wxColourProperty((selected->Type() == COMPONENT_HUD ? "Background" : "Color"), "ID_COLOR", Utils::ToWxColour(selected->Color)));
		}

		if (selected->Type() == COMPONENT_HUD)
		{
			// MATERIAL
			this->addPropertyRange("Opacity (alpha)", "ID_OPACITY",      (selected->Color.a * 255.0f), 0.0f, 255.0f, 1.0f);
			this->addPropertyCheckbox("Transparency", "ID_TRANSPARENCY", dynamic_cast<HUD*>(selected->Parent)->Transparent);

			// TEXT
			this->propertyGrid->Append(new wxPropertyCategory("Text"));

			this->propertyGrid->Append(new wxStringProperty("Text", "ID_TEXT", "HUD"));
			this->addPropertyEnum("Alignment", "ID_TEXT_ALIGNMENT", Utils::ALIGNMENTS, dynamic_cast<HUD*>(selected->Parent)->TextAlign);
			this->addPropertyEnum("Font",      "ID_TEXT_FONT",      Utils::FONTS,      dynamic_cast<HUD*>(selected->Parent)->TextFont);
			this->addPropertyRange("Size",     "ID_TEXT_SIZE",      dynamic_cast<HUD*>(selected->Parent)->TextSize, 10.0f, 100.0f, 1.0f);
			this->propertyGrid->Append(new wxColourProperty("Color", "ID_TEXT_COLOR",  dynamic_cast<HUD*>(selected->Parent)->TextColor));
			
			// TEXTURE
			this->propertyGrid->Append(new wxPropertyCategory("Texture"));

			this->propertyGrid->Append(new wxImageFileProperty("Texture", "ID_HUD_TEXTURE", selected->Textures[0]->ImageFile()));
			this->propertyGrid->SetPropertyAttribute("ID_HUD_TEXTURE", wxPG_FILE_WILDCARD, Utils::IMAGE_FILE_FORMATS);

			this->addPropertyCheckbox(" Remove Texture", "ID_HUD_REMOVE_TEXTURE", false);
		}
		else if (selected->Type() != COMPONENT_CAMERA)
		{
			// TEXTURE
			wxString label, imageFile;
			wxString skyboxLabels[]  = { "Right", "Left", "Top", "Bottom", "Back", "Front" };
			wxString terrainLabels[] = { "Background", "Red",  "Green", "Blue", "Blend Map", "" };
			wxString waterLabels[]   = { "", "", "DU/DV Map", "Normal Map", "", "" };

			for (int i = 0; i < MAX_TEXTURES; i++)
			{
				if (selected->Type() == COMPONENT_SKYBOX)
				{
					label     = skyboxLabels[i];
					imageFile = selected->Textures[0]->ImageFile(i);
				}
				else if (selected->Type() == COMPONENT_TERRAIN)
				{
					if (selected->Textures[i]->ImageFile(i).empty())
						continue;

					label     = terrainLabels[i];
					imageFile = selected->Textures[i]->ImageFile(0);
				}
				else if (selected->Type() == COMPONENT_WATER)
				{
					if (selected->Textures[i]->ImageFile(i).empty())
						continue;

					label     = waterLabels[i];
					imageFile = selected->Textures[i]->ImageFile(0);
				}
				else
				{
					if ((i > 0) || (selected->Textures[0] == nullptr))
						continue;

					label     = "Texture";
					imageFile = selected->Textures[0]->ImageFile(0);
				}

				this->propertyGrid->Append(new wxPropertyCategory("Texture"));
				
				this->propertyGrid->Append(new wxImageFileProperty(label, wxString("ID_TEXTURE_" + std::to_string(i)), imageFile));
				this->propertyGrid->SetPropertyAttribute(wxString("ID_TEXTURE_" + std::to_string(i)), wxPG_FILE_WILDCARD, Utils::IMAGE_FILE_FORMATS);

				this->addPropertyCheckbox(" Remove Texture", wxString("ID_REMOVE_TEXTURE_" + std::to_string(i)), false);

				if ((selected->Type() != COMPONENT_SKYBOX) && (selected->Type() != COMPONENT_TERRAIN) && (selected->Type() != COMPONENT_WATER)) {
					this->addPropertyCheckbox(" Flip Y", wxString("ID_FLIP_TEXTURE_"   + std::to_string(i)), selected->Textures[i]->FlipY());
					this->addPropertyCheckbox(" Repeat", wxString("ID_REPEAT_TEXTURE_" + std::to_string(i)), selected->Textures[i]->Repeat());
				}

				this->addPropertyRange(" Tiling U", wxString("ID_TILING_U_" + std::to_string(i)), selected->Textures[i]->Scale[0], 1.0f, 100.0f, 0.01f);
				this->addPropertyRange(" Tiling V", wxString("ID_TILING_V_" + std::to_string(i)), selected->Textures[i]->Scale[1], 1.0f, 100.0f, 0.01f);
			}
		}

		// PHYSICS
		if ((selected->Type() != COMPONENT_CAMERA) && (selected->Type() != COMPONENT_HUD) && (selected->Type() != COMPONENT_SKYBOX) && (selected->Type() != COMPONENT_TERRAIN) && (selected->Type() != COMPONENT_WATER))
		{
			BoundingVolume*    volume     = dynamic_cast<Mesh*>(selected)->GetBoundingVolume();
			BoundingVolumeType volumeType = (volume != nullptr ? volume->VolumeType() : BOUNDING_VOLUME_NONE);

			this->propertyGrid->Append(new wxPropertyCategory("Physics"));
			this->addPropertyEnum("Bounding Volume", "ID_BOUNDING_VOLUME", Utils::BOUNDING_VOLUMES, Utils::BOUNDING_VOLUMES[volumeType]);
		}

		// TERRAIN
		if (selected->Type() == COMPONENT_TERRAIN)
		{
			this->propertyGrid->Append(new wxPropertyCategory("Terrain"));

			this->addPropertyRange("Size",           "ID_TERRAIN_SIZE",           dynamic_cast<Terrain*>(selected->Parent)->Size(),           5.0f,  100.0f, 1.0f);
			this->addPropertyRange("Octaves",        "ID_TERRAIN_OCTAVES",        dynamic_cast<Terrain*>(selected->Parent)->Octaves(),        1.0f,  10.0f,  1.0f);
			this->addPropertyRange("Redistribution", "ID_TERRAIN_REDISTRIBUTION", dynamic_cast<Terrain*>(selected->Parent)->Redistribution(), 0.01f, 10.0f,  0.01f);
		}

		// WATER
		if (selected->Type() == COMPONENT_WATER)
		{
			WaterFBO* fbo = dynamic_cast<Water*>(selected->Parent)->FBO();

			if (fbo != nullptr)
			{
				this->propertyGrid->Append(new wxPropertyCategory("Water"));

				this->addPropertyRange("Speed",         "ID_WATER_SPEED",         fbo->Speed,        0.01f, 0.5f, 0.01f);
				this->addPropertyRange("Wave Strength", "ID_WATER_WAVE_STRENGTH", fbo->WaveStrength, 0.01f, 1.0f, 0.01f);
			}
		}

		this->propertyGrid->ExpandAll(false);

		if (selected->Type() != COMPONENT_SKYBOX) {
			this->propertyGrid->Expand("Location");
		}
	}

	this->sizerSceneDetails->Layout();
}

bool WindowFrame::IsDetailsActive()
{
	return this->propertyGrid->IsEditorFocused();
}

void WindowFrame::OnAbout(wxCommandEvent &event)
{
	wxString title = wxString(Utils::APP_NAME).append(" v.").append(Utils::APP_VERSION);
	wxString about = wxString(title).append("\n").append(Utils::COPYRIGHT).append(
		"\n\n"
		"Keyboard WASD:                                Move forward / left / back / right.\n"
		"Mouse Middle/Right:                        Rotate horizontal / vertical (yaw / pitch).\n"
		"Mouse Middle/Right + CTRL-key:  Move forward / back.\n"
		"Mouse Middle/Right + SHIFT-key: Move horizontal / vertical.\n"
		"Mouse Scroll:                                      Move forward / back.\n"
		"Mouse Scroll + CTRL-key:                Move left / right.\n"
		"Mouse Scroll + SHIFT-key:               Move up / down."
	).append("\n\n").append(Utils::TESTED);

	wxMessageBox(about, title, wxOK | wxICON_INFORMATION);
}

void WindowFrame::OnExit(wxCommandEvent &event)
{
	this->Close(true);
}

void WindowFrame::RemoveComponent(int index)
{
	if ((index > 0) && (index < (int)this->listBoxComponents->GetCount())) {
		this->listBoxComponents->Delete(index);
		this->listBoxComponents->Select(index - 1);
	}
}

void WindowFrame::RemoveChild(int index)
{
	if ((index >= 0) && (index < (int)this->listBoxChildren->GetCount()))
	{
		this->listBoxChildren->Delete(index);

		if (!this->listBoxChildren->IsEmpty())
			this->listBoxChildren->Select(std::max(index - 1, 0));
	}
}

void WindowFrame::SelectComponent(int index)
{
	if ((index >= 0) && (index < (int)this->listBoxComponents->GetCount())) {
		this->listBoxComponents->Select(index);
		this->AddListChildren(SceneManager::SelectedComponent->Children);
	}
}

void WindowFrame::SelectChild(int index)
{
	if ((index >= 0) && (index < (int)this->listBoxChildren->GetCount()))
		this->listBoxChildren->Select(index);
}

wxString WindowFrame::SelectedDrawMode()
{
	return this->dropDownDrawModes->GetStringSelection();
}

void WindowFrame::SetGraphicsAPI(int index)
{
	if ((index >= 0) && (index < (int)this->dropDownGraphicsAPIs->GetCount()))
		this->dropDownGraphicsAPIs->Select(index);
}

void WindowFrame::SetCanvas(wxGLCanvas* canvas)
{
	if (this->sizerMiddle->GetItemCount() > 1)
		this->sizerMiddle->Detach(0);

	wxSizerItem* canvasItem = this->sizerMiddle->Prepend(canvas);

	this->sizerMiddle->Layout();

	RenderEngine::Canvas.Position = canvasItem->GetPosition();
}

void WindowFrame::setPropertyXYZ(const wxString &id, float x, float y, float z)
{
	this->propertyGrid->SetPropertyValue(wxString(id + "_X"), x);
	this->propertyGrid->SetPropertyValue(wxString(id + "_Y"), y);
	this->propertyGrid->SetPropertyValue(wxString(id + "_Z"), z);
}

glm::vec3 WindowFrame::updatePropertyXYZ(const wxString &id, float value, const glm::vec3 &values)
{
	glm::vec3 xyz;

	if (id.substr(id.size() - 2) == "_X")
		xyz = glm::vec3(value, values[1], values[2]);
	else if (id.substr(id.size() - 2) == "_Y")
		xyz = glm::vec3(values[0], value, values[2]);
	else if (id.substr(id.size() - 2) == "_Z")
		xyz = glm::vec3(values[0], values[1], value);

	return xyz;
}

int WindowFrame::UpdateComponents(wxPGProperty* property)
{
	Component* selected = (SceneManager::SelectedChild != nullptr ? SceneManager::SelectedChild : SceneManager::SelectedComponent);

	if (selected == nullptr)
		return -1;

	wxString propertyName = property->GetName();

	if (propertyName == "ID_BOUNDING_VOLUME")
		dynamic_cast<Mesh*>(selected)->SetBoundingVolume(static_cast<BoundingVolumeType>(property->GetChoiceSelection()));
	else if (propertyName == "ID_COLOR")
		selected->Color = Utils::ToVec4Color(Utils::ToWxColour(property->GetValue()));
	else if (propertyName == "ID_ENABLE_AUTO_ROTATION")
		selected->AutoRotate = property->GetValue().GetBool();
	else if (propertyName == "ID_NAME")
		selected->Name = (!property->GetValueAsString().Trim().IsEmpty() ? property->GetValueAsString() : selected->Name);
	else if (propertyName == "ID_OPACITY")
		selected->Color.a = (property->GetValue().GetDouble() / 255.0f);
	else if (propertyName == "ID_TRANSPARENCY")
		dynamic_cast<HUD*>(selected->Parent)->Transparent = property->GetValue().GetBool();
	else if (propertyName == "ID_TEXT")
		dynamic_cast<HUD*>(selected->Parent)->Update(property->GetValueAsString());
	else if (propertyName == "ID_TEXT_ALIGNMENT")
		dynamic_cast<HUD*>(selected->Parent)->TextAlign = property->GetValueAsString();
	else if (propertyName == "ID_TEXT_FONT")
		dynamic_cast<HUD*>(selected->Parent)->TextFont = property->GetValueAsString();
	else if (propertyName == "ID_TEXT_SIZE")
		dynamic_cast<HUD*>(selected->Parent)->TextSize = property->GetValue().GetInteger();
	else if (propertyName == "ID_TEXT_COLOR")
		dynamic_cast<HUD*>(selected->Parent)->TextColor = Utils::ToWxColour(property->GetValue());
	else if (propertyName == "ID_HUD_TEXTURE")
		dynamic_cast<Mesh*>(selected)->LoadTextureImage(property->GetValueAsString(), 0);
	else if (propertyName == "ID_HUD_REMOVE_TEXTURE")
		dynamic_cast<Mesh*>(selected)->RemoveTexture(0);
	else if (propertyName == "ID_TERRAIN_SIZE")
		dynamic_cast<Terrain*>(selected->Parent)->Resize(property->GetValue().GetInteger(), dynamic_cast<Terrain*>(selected->Parent)->Octaves(), dynamic_cast<Terrain*>(selected->Parent)->Redistribution());
	else if (propertyName == "ID_TERRAIN_OCTAVES")
		dynamic_cast<Terrain*>(selected->Parent)->Resize(dynamic_cast<Terrain*>(selected->Parent)->Size(), property->GetValue().GetInteger(), dynamic_cast<Terrain*>(selected->Parent)->Redistribution());
	else if (propertyName == "ID_TERRAIN_REDISTRIBUTION")
		dynamic_cast<Terrain*>(selected->Parent)->Resize(dynamic_cast<Terrain*>(selected->Parent)->Size(), dynamic_cast<Terrain*>(selected->Parent)->Octaves(), property->GetValue().GetDouble());
	else if (propertyName == "ID_WATER_SPEED")
		dynamic_cast<Water*>(selected->Parent)->FBO()->Speed = property->GetValue().GetDouble();
	else if (propertyName == "ID_WATER_WAVE_STRENGTH")
		dynamic_cast<Water*>(selected->Parent)->FBO()->WaveStrength = property->GetValue().GetDouble();
	else if (propertyName.substr(0, 17) == "ID_AUTO_ROTATION_")
		selected->AutoRotation = this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->AutoRotation);
	else if (propertyName.substr(0, 16) == "ID_FLIP_TEXTURE_")
		dynamic_cast<Mesh*>(selected)->Textures[std::atoi(propertyName.substr(16))]->SetFlipY(property->GetValue().GetBool());
	else if (propertyName.substr(0, 12) == "ID_LOCATION_")
		selected->MoveTo(this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->Position()));
	else if (propertyName.substr(0, 18) == "ID_REMOVE_TEXTURE_")
		dynamic_cast<Mesh*>(selected)->RemoveTexture(std::atoi(propertyName.substr(18)));
	else if (propertyName.substr(0, 18) == "ID_REPEAT_TEXTURE_")
		dynamic_cast<Mesh*>(selected)->Textures[std::atoi(propertyName.substr(18))]->SetRepeat(property->GetValue().GetBool());
	else if (propertyName.substr(0, 12) == "ID_ROTATION_")
		selected->RotateTo(this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->Rotation()));
	else if (propertyName.substr(0, 11) == "ID_TILING_U_")
		dynamic_cast<Mesh*>(selected)->Textures[std::atoi(propertyName.substr(11))]->Scale[0] = property->GetValue().GetDouble();
	else if (propertyName.substr(0, 11) == "ID_TILING_V_")
		dynamic_cast<Mesh*>(selected)->Textures[std::atoi(propertyName.substr(11))]->Scale[1] = property->GetValue().GetDouble();
	else if (propertyName.substr(0, 9) == "ID_SCALE_")
		selected->ScaleTo(this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->Scale()));
	else if (propertyName.substr(0, 11) == "ID_TEXTURE_")
		dynamic_cast<Mesh*>(selected)->LoadTextureImage(property->GetValueAsString(), std::atoi(propertyName.substr(11)));

	if (selected->Type() == COMPONENT_HUD)
		dynamic_cast<HUD*>(selected->Parent)->Update();

	return this->UpdateDetails();
}

int WindowFrame::UpdateDetails()
{
	Component* selected = (SceneManager::SelectedChild != nullptr ? SceneManager::SelectedChild : SceneManager::SelectedComponent);

	if ((selected == nullptr) || (this->propertyGrid == nullptr))
		return -1;

	this->propertyGrid->SetPropertyValue("ID_NAME", static_cast<wxString>(selected->Name));

	// UPDATE LIST OF COMPONENTS/CHILDREN
	if (selected->Type() == COMPONENT_CAMERA)
		this->listBoxComponents->SetString(0, selected->Name);
	else
		this->SelectComponent(this->listBoxComponents->GetSelection());

	if (selected->Type() != COMPONENT_SKYBOX) {
		glm::vec3 position = selected->Position();
		this->setPropertyXYZ("ID_LOCATION", position[0], position[1], position[2]);
	}

	if (selected->Type() != COMPONENT_SKYBOX) {
		glm::vec3 rotation = selected->Rotation();
		this->setPropertyXYZ("ID_ROTATION", rotation[0], rotation[1], rotation[2]);
	}

	if ((selected->Type() != COMPONENT_CAMERA) && (selected->Type() != COMPONENT_SKYBOX)) {
		glm::vec3 scale = selected->Scale();
		this->setPropertyXYZ("ID_SCALE", scale[0], scale[1], scale[2]);
	}

	if ((selected->Type() != COMPONENT_CAMERA) && (selected->Type() != COMPONENT_SKYBOX))
	{
		glm::vec3 autoRotation = selected->AutoRotation;

		this->setPropertyXYZ("ID_AUTO_ROTATION", autoRotation[0], autoRotation[1], autoRotation[2]);
		this->propertyGrid->SetPropertyValue("ID_ENABLE_AUTO_ROTATION", selected->AutoRotate);
	}

	if ((selected->Type() != COMPONENT_CAMERA) && (selected->Type() != COMPONENT_SKYBOX))
		this->propertyGrid->SetPropertyValue("ID_COLOR", Utils::ToWxColour(selected->Color));

	if (selected->Type() == COMPONENT_HUD)
	{
		this->propertyGrid->SetPropertyValue("ID_OPACITY",            (selected->Color.a * 255.0f));
		this->propertyGrid->SetPropertyValue("ID_TRANSPARENCY",       dynamic_cast<HUD*>(selected->Parent)->Transparent);
		this->propertyGrid->SetPropertyValue("ID_TEXT",               dynamic_cast<HUD*>(selected->Parent)->Text());
		this->propertyGrid->SetPropertyValue("ID_TEXT_ALIGNMENT",     dynamic_cast<HUD*>(selected->Parent)->TextAlign);
		this->propertyGrid->SetPropertyValue("ID_TEXT_FONT",          dynamic_cast<HUD*>(selected->Parent)->TextFont);
		this->propertyGrid->SetPropertyValue("ID_TEXT_SIZE",          dynamic_cast<HUD*>(selected->Parent)->TextSize);
		this->propertyGrid->SetPropertyValue("ID_TEXT_COLOR",         dynamic_cast<HUD*>(selected->Parent)->TextColor);
		this->propertyGrid->SetPropertyValue("ID_HUD_TEXTURE",        selected->Textures[0]->ImageFile());
		this->propertyGrid->SetPropertyValue("ID_HUD_REMOVE_TEXTURE", false);
	}
	else if (selected->Type() != COMPONENT_CAMERA)
	{
		for (int i = 0; i < MAX_TEXTURES; i++)
		{
			if (selected->Type() == COMPONENT_TERRAIN) {
				if (selected->Textures[i]->ImageFile(i).empty())
					continue;
			} else if (selected->Type() == COMPONENT_WATER) {
				if (selected->Textures[i]->ImageFile(i).empty())
					continue;
			} else {
				if (i > 0)
					continue;
			}

			this->propertyGrid->SetPropertyValue(wxString("ID_TEXTURE_"        + std::to_string(i)), selected->Textures[i]->ImageFile());
			this->propertyGrid->SetPropertyValue(wxString("ID_REMOVE_TEXTURE_" + std::to_string(i)), false);

			if ((selected->Type() != COMPONENT_SKYBOX) && (selected->Type() != COMPONENT_TERRAIN) && (selected->Type() != COMPONENT_WATER)) {
				this->propertyGrid->SetPropertyValue(wxString("ID_FLIP_TEXTURE_"   + std::to_string(i)), selected->Textures[i]->FlipY());
				this->propertyGrid->SetPropertyValue(wxString("ID_REPEAT_TEXTURE_" + std::to_string(i)), selected->Textures[i]->Repeat());
			}

			this->propertyGrid->SetPropertyValue(wxString("ID_TILING_U_" + std::to_string(i)), selected->Textures[i]->Scale[0]);
			this->propertyGrid->SetPropertyValue(wxString("ID_TILING_V_" + std::to_string(i)), selected->Textures[i]->Scale[1]);
		}
	}

	if ((selected->Type() != COMPONENT_CAMERA) && (selected->Type() != COMPONENT_HUD) && (selected->Type() != COMPONENT_SKYBOX) && (selected->Type() != COMPONENT_TERRAIN) && (selected->Type() != COMPONENT_WATER))
	{
		BoundingVolume*    volume     = dynamic_cast<Mesh*>(selected)->GetBoundingVolume();
		BoundingVolumeType volumeType = (volume != nullptr ? volume->VolumeType() : BOUNDING_VOLUME_NONE);

		this->propertyGrid->SetPropertyValue("ID_BOUNDING_VOLUME", volumeType);
	}

	if (selected->Type() == COMPONENT_TERRAIN)
	{
		this->propertyGrid->SetPropertyValue("ID_TERRAIN_SIZE",           dynamic_cast<Terrain*>(selected->Parent)->Size());
		this->propertyGrid->SetPropertyValue("ID_TERRAIN_OCTAVES",        dynamic_cast<Terrain*>(selected->Parent)->Octaves());
		this->propertyGrid->SetPropertyValue("ID_TERRAIN_REDISTRIBUTION", dynamic_cast<Terrain*>(selected->Parent)->Redistribution());
	}

	if (selected->Type() == COMPONENT_WATER)
	{
		WaterFBO* fbo = dynamic_cast<Water*>(selected->Parent)->FBO();

		if (fbo != nullptr) {
			this->propertyGrid->SetPropertyValue("ID_WATER_SPEED",         fbo->Speed);
			this->propertyGrid->SetPropertyValue("ID_WATER_WAVE_STRENGTH", fbo->WaveStrength);
		}
	}

	return 0;
}
