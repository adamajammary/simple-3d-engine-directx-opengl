#include "WindowFrame.h"

WindowFrame::WindowFrame(const wxString &title, const wxPoint &pos, const wxSize &size, Window* parent) : wxFrame(NULL, wxID_ANY, title, pos, size)
{
	this->Parent = parent;

	this->init();
	//int result = this->init();

	//// TODO: Handle window init errors
	//if (result < 0)
	//	//
}

void WindowFrame::addAds(wxBoxSizer* sizer)
{
	// ADS
	wxStaticBoxSizer* sizerAds = new wxStaticBoxSizer(wxHORIZONTAL, this->Parent->GetTopWindow(), "Google Ads");
	wxWebView*        webView  = nullptr;

	webView = wxWebView::New(this, wxID_ANY, Utils::GOOGLE_ADS_URL, wxDefaultPosition, Utils::UI_ADS_SIZE, wxWebViewBackendDefault, wxBORDER_NONE);
	sizerAds->Add(webView, 0, wxALL, 10);

	sizer->Add(sizerAds, 0, (wxALIGN_CENTER_HORIZONTAL | wxALL), 10);
}

void WindowFrame::addButton(wxWindow* parent, void(*eventHandler)(wxCommandEvent &e), wxBoxSizer* sizer, IconType id, wxString label, int flag, const wxPoint &position, const wxSize &size)
{
	wxButton* button = new wxButton(parent, id, label, position, size);

	button->Bind(wxEVT_BUTTON, eventHandler, id, id);
	sizer->Add(button, 0, flag, 10);
}

wxCheckBox* WindowFrame::addCheckBox(wxGridBagSizer* sizer, IconType id, wxGBPosition position, bool default)
{
	wxCheckBox* checkBox = new wxCheckBox(this, id, "");

	checkBox->Bind(wxEVT_CHECKBOX, &InputManager::OnGraphicsMenu, id, id);
	checkBox->SetValue(default);
	sizer->Add(checkBox, position, wxDefaultSpan, (wxALIGN_CENTER_VERTICAL | wxLEFT), 10);

	return checkBox;
}

wxChoice* WindowFrame::addDropDown(wxGridBagSizer* sizer, IconType id, wxGBPosition position, int nrOfChoices, const wxString* choices)
{
	wxChoice* dropDown = new wxChoice(this, id, wxDefaultPosition, wxDefaultSize, nrOfChoices, choices);

	dropDown->Select(0);
	dropDown->Bind(wxEVT_CHOICE, &InputManager::OnGraphicsMenu, id, id);

	sizer->Add(dropDown, position, wxDefaultSpan, wxLEFT, 10);

	return dropDown;
}

void WindowFrame::addIcon(wxWindow* parent, wxBoxSizer* sizer, const Icon &icon, int flag)
{
	wxImage image = wxImage(icon.File, wxBITMAP_TYPE_PNG);
	image.Rescale(30, 30, wxIMAGE_QUALITY_BICUBIC);

	wxButton* button = new wxButton(parent, icon.ID, icon.Title, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

	wxFont font = button->GetFont();
	font.SetWeight(wxFONTWEIGHT_BOLD);
	button->SetFont(font);

	button->SetForegroundColour(wxColour(0xFF, 0xFF, 0xFF, 0xFF));
	button->SetBackgroundColour(wxColour(0x20, 0x20, 0x20, 0xFF));
	button->SetBitmap(image);

	button->Bind(wxEVT_BUTTON, &InputManager::OnIcon, icon.ID, icon.ID);

	sizer->Add(button, 0, flag, 9);
}

void WindowFrame::addLine(wxBoxSizer* sizer, int flag, wxSize size)
{
	wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, size, wxLI_HORIZONTAL);

	line->SetForegroundColour(wxColour(0xFF, 0xFF, 0xFF, 0xFF));
	sizer->Add(line, 0, flag, 10);
}

wxListBox* WindowFrame::addListBox(wxBoxSizer* sizer, IconType id)
{
	wxListBox* listBox = new wxListBox(this, id, wxDefaultPosition, Utils::UI_LIST_BOX_SIZE, 0, 0, (wxLB_SINGLE | wxLB_ALWAYS_SB));

	listBox->Bind(wxEVT_LISTBOX, &InputManager::OnList, id, id);
	sizer->Add(listBox, 0, wxLEFT, 10);

	return listBox;
}

void WindowFrame::AddListComponent(Component* component)
{
	this->listBoxComponents->AppendString(component->Name);
	this->SelectComponent(this->listBoxComponents->GetCount() - 1);
}

void WindowFrame::AddListChildren(std::vector<Component*> children)
{
	this->listBoxChildren->Clear();

	for (auto child : children) {
		SceneManager::SelectedChild = child;
		this->listBoxChildren->AppendString(child->Name);
	}

	this->listBoxChildren->Select(this->listBoxChildren->GetCount() - 1);
}

void WindowFrame::addMenu()
{
	wxMenu* menuFile        = new wxMenu;
	wxMenu* menuEnvironment = new wxMenu;
	wxMenu* menuGeometry    = new wxMenu;
	wxMenu* menuLights      = new wxMenu;
	wxMenu* menuUI          = new wxMenu;
	wxMenu* menuHelp        = new wxMenu;

	// FILE
	menuFile->Append(ID_SCENE_LOAD,  "Load Scene");
	menuFile->Append(ID_SCENE_SAVE,  "Save Scene");
	menuFile->Append(ID_SCENE_CLEAR, "Clear Scene");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	// ENVIRONMENT
	for (auto icon : Utils::ICONS_ENVIRONMENT)
		menuEnvironment->Append(icon.ID, icon.Title);

	// GEOMETRY
	menuGeometry->Append(ID_ICON_BROWSE, "Browse");
	menuGeometry->AppendSeparator();

	for (auto icon : Utils::ICONS_GEOMETRY)
		menuGeometry->Append(icon.ID, icon.Title);

	// LIGHTS
	for (auto icon : Utils::ICONS_LIGHTS)
		menuLights->Append(icon.ID, icon.Title);

	// UI
	for (auto icon : Utils::ICONS_UI)
		menuUI->Append(icon.ID, icon.Title);

	// HELP
	menuHelp->Append(wxID_ABOUT);

	// MENU
	wxMenuBar* menuBar = new wxMenuBar;

	menuBar->Append(menuFile,        "&File");
	menuBar->Append(menuEnvironment, "&Environment");
	menuBar->Append(menuGeometry,    "&Geometry");
	menuBar->Append(menuLights,      "&Lights");
	menuBar->Append(menuUI,          "&UI");
	menuBar->Append(menuHelp,        "&Help");

	this->SetMenuBar(menuBar);
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

void WindowFrame::addTab(IconType id, const wxString &label, wxNotebook* tabs, const std::vector<Icon> &icons)
{
	wxNotebookPage* tab   = new wxNotebookPage(tabs, id);
	wxBoxSizer*     sizer = new wxBoxSizer(wxHORIZONTAL);

	if (id == ID_TABS_GEOMETRY)
	{
		this->addButton(
			tab, InputManager::OnIcon, sizer, ID_ICON_BROWSE, "BROWSE",
			(wxLEFT | wxTOP), wxDefaultPosition, wxSize(-1, 36)
		);
	}

	for (auto icon : icons)
		this->addIcon(tab, sizer, icon, (wxLEFT | wxTOP));

	tab->SetBackgroundColour(this->GetBackgroundColour());
	tab->SetSizer(sizer);
	tabs->AddPage(tab, label);
}

wxStaticText* WindowFrame::addTextLabel(wxString text)
{
	wxStaticText* label = new wxStaticText(this, wxID_ANY, text);

	wxFont font = label->GetFont();
	font.SetWeight(wxFONTWEIGHT_BOLD);
	label->SetFont(font);

	return label;
}

void WindowFrame::addTextLabel(wxGridBagSizer* sizer, wxString text, wxGBPosition position, int flag, int border)
{
	sizer->Add(this->addTextLabel(text), position, wxDefaultSpan, flag, border);
}

void WindowFrame::addTextLabel(wxBoxSizer* sizer, wxString text, int flag, int border)
{
	sizer->Add(this->addTextLabel(text), 0, flag, border);
}

void WindowFrame::ClearScene()
{
	this->listBoxComponents->Clear();
	this->listBoxChildren->Clear();
}

void WindowFrame::DeactivateProperties()
{
	this->propertyGrid->UnfocusEditor();
}

int WindowFrame::init()
{
	// ICON
	wxIcon icon = wxIcon("img/icon_64.ico", wxBITMAP_TYPE_ICO);
	this->SetIcon(icon);

	// MENU
	this->addMenu();

	// INIT
	this->SetForegroundColour(wxColour(0xFF, 0xFF, 0xFF, 0xFF));
	this->SetBackgroundColour(wxColour(0x20, 0x20, 0x20, 0xFF));
	this->Centre();

	// MAIN
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(sizer);

	// ADS
	this->addAds(sizer);

	// TABS
	wxNotebook* tabs = new wxNotebook(this, ID_TABS, wxDefaultPosition, Utils::UI_TABS_SIZE, wxNB_TOP);

	this->addTab(ID_TABS_GEOMETRY, "Geometry",    tabs, Utils::ICONS_GEOMETRY);
	this->addTab(ID_TABS_LIGHTS,   "Environment", tabs, Utils::ICONS_ENVIRONMENT);
	this->addTab(ID_TABS_LIGHTS,   "Lights",      tabs, Utils::ICONS_LIGHTS);
	this->addTab(ID_TABS_LIGHTS,   "UI",          tabs, Utils::ICONS_UI);

	tabs->Layout();
	sizer->Add(tabs, 0, wxLEFT, 10);

	// TOP - DROPDOWNS
	wxGridBagSizer* sizerDropDowns = new wxGridBagSizer();

	// GRAPHICS API
	this->addTextLabel(sizerDropDowns, "Graphics API:", wxGBPosition(1, 1), wxALIGN_CENTER_VERTICAL);
	this->dropDownGraphicsAPIs = this->addDropDown(sizerDropDowns, ID_GRAPHICS_API, wxGBPosition(1, 2), 4, Utils::GRAPHIC_APIS);

	// ASPECT RATIO
	this->addTextLabel(sizerDropDowns, "Aspect Ratio:", wxGBPosition(1, 3), (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);
	this->addDropDown(sizerDropDowns, ID_ASPECT_RATIO, wxGBPosition(1, 4), 2, Utils::ASPECT_RATIOS);

	// FOV
	this->addTextLabel(sizerDropDowns, "FOV:", wxGBPosition(1, 5), (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);
	this->addDropDown(sizerDropDowns, ID_FOV, wxGBPosition(1, 6), 4, Utils::FOVS);

	// DRAW MODE
	this->addTextLabel(sizerDropDowns, "Draw Mode:", wxGBPosition(1, 7), (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);
	this->dropDownDrawModes = this->addDropDown(sizerDropDowns, ID_DRAW_MODE, wxGBPosition(1, 8), 2, Utils::DRAW_MODES);

	// DRAW BOUNDING VOLUMES
	this->addTextLabel(sizerDropDowns, "Draw Bounding Volumes:", wxGBPosition(1, 9), (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);
	this->addCheckBox(sizerDropDowns, ID_DRAW_BOUNDING, wxGBPosition(1, 10));

	// VSYNC
	this->addTextLabel(sizerDropDowns, "V-sync:", wxGBPosition(1, 11), (wxALIGN_CENTER_VERTICAL | wxLEFT), 20);
	this->VSyncEnable = this->addCheckBox(sizerDropDowns, ID_VSYNC, wxGBPosition(1, 12), true);

	sizer->Add(sizerDropDowns, 0, wxBOTTOM, 10);

	// MIDDLE
	this->sizerMiddle = new wxBoxSizer(wxHORIZONTAL);

	// MIDDLE-LEFT - CANVAS

	// MIDDLE-RIGHT
	wxBoxSizer* sizerMiddleRight = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerSceneMenu   = new wxBoxSizer(wxHORIZONTAL);

	this->addTextLabel(sizerSceneMenu, "Scene", (wxALIGN_CENTER_VERTICAL | wxLEFT), 10);

	this->addButton(this, InputManager::OnList, sizerSceneMenu, ID_SCENE_SAVE,  "SAVE",  wxLEFT);
	this->addButton(this, InputManager::OnList, sizerSceneMenu, ID_SCENE_LOAD,  "LOAD",  wxLEFT);
	this->addButton(this, InputManager::OnList, sizerSceneMenu, ID_SCENE_CLEAR, "CLEAR", wxLEFT);

	sizerMiddleRight->Add(sizerSceneMenu, 0, wxBOTTOM, 10);

	wxBoxSizer* sizerSceneComponents = new wxBoxSizer(wxHORIZONTAL);

	// COMPONENTS AND CHILDREN
	this->listBoxComponents = this->addListBox(sizerSceneComponents, ID_COMPONENTS);
	this->listBoxChildren   = this->addListBox(sizerSceneComponents, ID_CHILDREN);

	sizerMiddleRight->Add(sizerSceneComponents);

	// PROPERTIES
	this->sizerSceneProperties = new wxBoxSizer(wxVERTICAL);

	sizerMiddleRight->Add(this->sizerSceneProperties, 1, (wxLEFT | wxTOP), 10);
	this->sizerMiddle->Add(sizerMiddleRight, 1, wxEXPAND);
	sizer->Add(this->sizerMiddle, 1, (wxLEFT | wxTOP), 10);

	// STATUS BAR
	this->CreateStatusBar();

	return 0;
}

int WindowFrame::InitProperties()
{
	this->sizerSceneProperties->Clear(true);

	wxBoxSizer*   sizerButtons = new wxBoxSizer(wxHORIZONTAL);
	ComponentType selectedType = SceneManager::SelectedComponent->Type();

	// REMOVE COMPONENT
	if ((SceneManager::SelectedComponent != nullptr) && (selectedType != COMPONENT_CAMERA))
		this->addButton(this, InputManager::OnList, sizerButtons, ID_REMOVE_COMPONENT, wxString("REMOVE " + SceneManager::SelectedComponent->Name), wxBOTTOM);

	// REMOVE CHILD
	if ((SceneManager::SelectedChild != nullptr) && (selectedType != COMPONENT_CAMERA))
		this->addButton(this, InputManager::OnList, sizerButtons, ID_REMOVE_CHILD, wxString("REMOVE " + SceneManager::SelectedChild->Name), wxLEFT);

	this->sizerSceneProperties->Add(sizerButtons);

	this->propertyGrid = new wxPropertyGrid(this, ID_SCENE_DETAILS, wxDefaultPosition, Utils::UI_PROPS_SIZE, wxPG_SPLITTER_AUTO_CENTER);
	this->propertyGrid->SetEmptySpaceColour(wxColour(0x40, 0x40, 0x40, 0xFF));
	this->sizerSceneProperties->Add(this->propertyGrid, 1, (wxEXPAND | wxRIGHT | wxBOTTOM), 10);
	this->propertyGrid->Bind(wxEVT_PG_CHANGED, &InputManager::OnPropertyChanged, ID_SCENE_DETAILS, ID_SCENE_DETAILS);

	Component* selected = (SceneManager::SelectedChild != nullptr ? SceneManager::SelectedChild : SceneManager::SelectedComponent);

	if ((selected != nullptr) && (this->propertyGrid != nullptr))
	{
		this->propertyGrid->Clear();

		// NAME
		this->propertyGrid->Append(new wxStringProperty("Name", Utils::PROPERTY_IDS[PROPERTY_ID_NAME], static_cast<wxString>(selected->Name)));
		this->propertyGrid->GetProperty(Utils::PROPERTY_IDS[PROPERTY_ID_NAME])->SetValidator(wxTextValidator(wxFILTER_EMPTY));

		if (selected->Type() == COMPONENT_CAMERA)
			this->propertyGrid->SetPropertyReadOnly(Utils::PROPERTY_IDS[PROPERTY_ID_NAME]);

		// LIGHT SOURCE
		if (selected->Type() == COMPONENT_LIGHTSOURCE)
			return this->initPropertiesLightSources(selected);

		if (selected->Type() != COMPONENT_SKYBOX)
		{
			// LOCATION / POSITION
			glm::vec3 position = selected->Position();
			this->addPropertyXYZ("Location", Utils::PROPERTY_IDS[PROPERTY_ID_LOCATION], position[0], position[1], position[2], -100.0f, 100.0f, 0.01f);

			// ROTATION
			glm::vec3 rotation = selected->Rotation();
			this->addPropertyXYZ("Rotation (rad)", Utils::PROPERTY_IDS[PROPERTY_ID_ROTATION], rotation[0], rotation[1], rotation[2], -glm::pi<float>(), glm::pi<float>(), 0.01f);
		}

		// CAMERA
		if (selected->Type() == COMPONENT_CAMERA)
		{
			this->propertyGrid->ExpandAll();
			this->sizerSceneProperties->Layout();

			return 0;
		}

		if (selected->Type() != COMPONENT_SKYBOX)
		{
			// SCALE
			glm::vec3 scale = selected->Scale();
			this->addPropertyXYZ("Scale", Utils::PROPERTY_IDS[PROPERTY_ID_SCALE], scale[0], scale[1], scale[2], -100.0f, 100.0f, 0.01f);

			// AUTO-ROTATE
			glm::vec3 autoRotation = selected->AutoRotation;
			this->addPropertyXYZ("Auto-Rotate (rad)", Utils::PROPERTY_IDS[PROPERTY_ID_AUTO_ROTATION], autoRotation[0], autoRotation[1], autoRotation[2], -glm::pi<float>(), glm::pi<float>(), 0.01f);
			this->addPropertyCheckbox("Enable", Utils::PROPERTY_IDS[PROPERTY_ID_ENABLE_AUTO_ROTATION], selected->AutoRotate);
			
			// MATERIAL
			this->propertyGrid->Append(new wxPropertyCategory("Material"));

			this->propertyGrid->Append(new wxColourProperty((selected->Type() == COMPONENT_HUD ? "Background" : "Color"), Utils::PROPERTY_IDS[PROPERTY_ID_COLOR], Utils::ToWxColour(selected->ComponentMaterial.diffuse)));
			this->propertyGrid->Append(new wxColourProperty("Specular Intensity", Utils::PROPERTY_IDS[PROPERTY_ID_SPEC_INTENSITY], Utils::ToWxColour(selected->ComponentMaterial.specular.intensity)));
			this->addPropertyRange("Specular Shininess", Utils::PROPERTY_IDS[PROPERTY_ID_SPEC_SHININESS], selected->ComponentMaterial.specular.shininess, 0.0f, 1024.0f, 1.0f);
		}

		if (selected->Type() == COMPONENT_HUD)
		{
			// MATERIAL
			this->addPropertyRange("Opacity (alpha)", Utils::PROPERTY_IDS[PROPERTY_ID_OPACITY],      (selected->ComponentMaterial.diffuse.a * 255.0f), 0.0f, 255.0f, 1.0f);
			this->addPropertyCheckbox("Transparency", Utils::PROPERTY_IDS[PROPERTY_ID_TRANSPARENCY], dynamic_cast<HUD*>(selected->Parent)->Transparent);

			// TEXT
			this->propertyGrid->Append(new wxPropertyCategory("Text"));

			this->propertyGrid->Append(new wxStringProperty("Text", Utils::PROPERTY_IDS[PROPERTY_ID_TEXT], "HUD"));
			this->addPropertyEnum("Alignment", Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_ALIGNMENT], Utils::ALIGNMENTS, dynamic_cast<HUD*>(selected->Parent)->TextAlign);
			this->addPropertyEnum("Font",      Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_FONT],      Utils::FONTS,      dynamic_cast<HUD*>(selected->Parent)->TextFont);
			this->addPropertyRange("Size",     Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_SIZE],      dynamic_cast<HUD*>(selected->Parent)->TextSize, 10.0f, 100.0f, 1.0f);
			this->propertyGrid->Append(new wxColourProperty("Color", Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_COLOR],  dynamic_cast<HUD*>(selected->Parent)->TextColor));
		}

		// TEXTURES
		this->initPropertiesTextures(selected);

		// SKYBOX
		if ((selected->Type() == COMPONENT_HUD) || (selected->Type() == COMPONENT_SKYBOX))
		{
			this->propertyGrid->ExpandAll();
			this->sizerSceneProperties->Layout();

			return 0;
		}

		// PHYSICS
		if ((selected->Type() != COMPONENT_TERRAIN) && (selected->Type() != COMPONENT_WATER))
		{
			BoundingVolume*    volume     = dynamic_cast<Mesh*>(selected)->GetBoundingVolume();
			BoundingVolumeType volumeType = (volume != nullptr ? volume->VolumeType() : BOUNDING_VOLUME_NONE);

			this->propertyGrid->Append(new wxPropertyCategory("Physics"));
			this->addPropertyEnum("Bounding Volume", Utils::PROPERTY_IDS[PROPERTY_ID_BOUNDING_VOLUME], Utils::BOUNDING_VOLUMES, Utils::BOUNDING_VOLUMES[volumeType]);
		}

		// TERRAIN
		if (selected->Type() == COMPONENT_TERRAIN)
		{
			this->propertyGrid->Append(new wxPropertyCategory("Terrain"));

			this->addPropertyRange("Size",           Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_SIZE],           dynamic_cast<Terrain*>(selected->Parent)->Size(),           5.0f,  100.0f, 1.0f);
			this->addPropertyRange("Octaves",        Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_OCTAVES],        dynamic_cast<Terrain*>(selected->Parent)->Octaves(),        1.0f,  10.0f,  1.0f);
			this->addPropertyRange("Redistribution", Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_REDISTRIBUTION], dynamic_cast<Terrain*>(selected->Parent)->Redistribution(), 0.01f, 10.0f,  0.01f);
		}

		// WATER
		if (selected->Type() == COMPONENT_WATER)
		{
			WaterFBO* fbo = dynamic_cast<Water*>(selected->Parent)->FBO();

			if (fbo != nullptr)
			{
				this->propertyGrid->Append(new wxPropertyCategory("Water"));

				this->addPropertyRange("Speed",         Utils::PROPERTY_IDS[PROPERTY_ID_WATER_SPEED],         fbo->Speed,        0.01f, 0.5f, 0.01f);
				this->addPropertyRange("Wave Strength", Utils::PROPERTY_IDS[PROPERTY_ID_WATER_WAVE_STRENGTH], fbo->WaveStrength, 0.01f, 1.0f, 0.01f);
			}
		}

		this->propertyGrid->ExpandAll();
	}

	this->sizerSceneProperties->Layout();

	return 0;
}

int WindowFrame::initPropertiesLightSources(Component* selected)
{
	Attenuation  attenuation;
	glm::vec3    direction;
	LightSource* lightSource = dynamic_cast<LightSource*>(selected->Parent);

	// TOGGLE ACTIVE STATE
	this->addPropertyCheckbox("Active", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ACTIVE], lightSource->Active());

	// LOCATION / POSITION
	glm::vec3 position = selected->Position();
	this->addPropertyXYZ("Location", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_LOCATION], position[0], position[1], position[2], -100.0f, 100.0f, 0.01f);

	switch (lightSource->SourceType()) {
	case ID_ICON_LIGHT_DIRECTIONAL:
		direction = lightSource->Direction();
		this->addPropertyXYZ("Direction", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_DIRECTION], direction[0], direction[1], direction[2], -1.0f, 1.0f);

		break;
	case ID_ICON_LIGHT_POINT:
		attenuation = lightSource->GetAttenuation();
		this->propertyGrid->Append(new wxPropertyCategory("Attenuation"));

		this->addPropertyRange("Attenuation Linear",    Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_LINEAR], attenuation.linear);
		this->addPropertyRange("Attenuation Quadratic", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_QUAD],   attenuation.quadratic, 0.0f, 2.0f);

		break;
	case ID_ICON_LIGHT_SPOT:
		direction = lightSource->Direction();
		this->addPropertyXYZ("Direction", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_DIRECTION], direction[0], direction[1], direction[2], -1.0f, 1.0f);

		attenuation = lightSource->GetAttenuation();
		this->propertyGrid->Append(new wxPropertyCategory("Attenuation"));

		this->addPropertyRange("Attenuation Linear",    Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_LINEAR], attenuation.linear);
		this->addPropertyRange("Attenuation Quadratic", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_QUAD],   attenuation.quadratic, 0.0f, 2.0f);

		this->propertyGrid->Append(new wxPropertyCategory("Cone"));

		this->addPropertyRange("Inner Angle (rad)", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ANGLE_INNER], lightSource->GetConeInnerAngle(), -glm::pi<float>(), glm::pi<float>(), 0.01f);
		this->addPropertyRange("Outer Angle (rad)", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ANGLE_OUTER], lightSource->GetConeOuterAngle(), -glm::pi<float>(), glm::pi<float>(), 0.01f);

		break;
	default:
		throw;
	}

	// MATERIAL
	Material material = lightSource->GetMaterial();
	this->propertyGrid->Append(new wxPropertyCategory("Material"));

	this->propertyGrid->Append(new wxColourProperty("Ambient",            Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_AMBIENT],        Utils::ToWxColour(material.ambient)));
	this->propertyGrid->Append(new wxColourProperty("Color",              Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_DIFFUSE],        Utils::ToWxColour(material.diffuse)));
	this->propertyGrid->Append(new wxColourProperty("Specular Intensity", Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_SPEC_INTENSITY], Utils::ToWxColour(material.specular.intensity)));
	this->addPropertyRange("Specular Shininess",                          Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_SPEC_SHININESS], material.specular.shininess, 0.0f, 1024.0f, 1.0f);

	this->propertyGrid->ExpandAll();
	this->sizerSceneProperties->Layout();

	return 0;
}

void WindowFrame::initPropertiesTextures(Component* selected)
{
	if (selected->Type() == COMPONENT_HUD)
	{
		this->propertyGrid->Append(new wxPropertyCategory("Texture"));

		this->propertyGrid->Append(new wxImageFileProperty("Texture", Utils::PROPERTY_IDS[PROPERTY_ID_HUD_TEXTURE], selected->Textures[0]->ImageFile()));
		this->propertyGrid->SetPropertyAttribute(Utils::PROPERTY_IDS[PROPERTY_ID_HUD_TEXTURE], wxPG_FILE_WILDCARD, Utils::IMAGE_FILE_FORMATS);

		this->addPropertyCheckbox(" Remove Texture", Utils::PROPERTY_IDS[PROPERTY_ID_HUD_REMOVE_TEXTURE], false);
	}
	else
	{
		wxString label, imageFile;
		wxString defaultLabels[] = { "Texture (diffuse)", "Texture (specular)", "", "", "", "" };
		wxString skyboxLabels[]  = { "Right", "Left", "Top", "Bottom", "Back", "Front" };
		wxString terrainLabels[] = { "Background", "Red",  "Green", "Blue", "Blend Map", "" };
		wxString waterLabels[]   = { "", "", "DU/DV Map", "Normal Map", "", "" };

		this->propertyGrid->Append(new wxPropertyCategory("Texture"));

		for (int i = 0; i < MAX_TEXTURES; i++)
		{
			if (selected->Type() == COMPONENT_SKYBOX)
			{
				label     = skyboxLabels[i];
				imageFile = selected->Textures[0]->ImageFile(i);
			}
			else if (selected->Type() == COMPONENT_TERRAIN)
			{
				if (selected->Textures[i]->ImageFile(0).empty())
					continue;

				label     = terrainLabels[i];
				imageFile = selected->Textures[i]->ImageFile(0);
			}
			else if (selected->Type() == COMPONENT_WATER)
			{
				if (selected->Textures[i]->ImageFile(0).empty())
					continue;

				label     = waterLabels[i];
				imageFile = selected->Textures[i]->ImageFile(0);
			}
			else
			{
				if (i > 1)
					continue;

				label     = defaultLabels[i];
				imageFile = selected->Textures[i]->ImageFile(0);
			}

			this->propertyGrid->Append(new wxImageFileProperty(label, wxString(Utils::PROPERTY_IDS[PROPERTY_ID_TEXTURE_] + std::to_string(i)), imageFile));
			this->propertyGrid->SetPropertyAttribute(wxString(Utils::PROPERTY_IDS[PROPERTY_ID_TEXTURE_] + std::to_string(i)), wxPG_FILE_WILDCARD, Utils::IMAGE_FILE_FORMATS);

			this->addPropertyCheckbox(" Remove Texture", wxString(Utils::PROPERTY_IDS[PROPERTY_ID_REMOVE_TEXTURE_] + std::to_string(i)), false);

			if ((selected->Type() != COMPONENT_SKYBOX) && (selected->Type() != COMPONENT_TERRAIN) && (selected->Type() != COMPONENT_WATER)) {
				this->addPropertyCheckbox(" Flip Y", wxString(Utils::PROPERTY_IDS[PROPERTY_ID_FLIP_TEXTURE_]   + std::to_string(i)), selected->Textures[i]->FlipY());
				this->addPropertyCheckbox(" Repeat", wxString(Utils::PROPERTY_IDS[PROPERTY_ID_REPEAT_TEXTURE_] + std::to_string(i)), selected->Textures[i]->Repeat());
			}

			this->addPropertyRange(" Tiling U", wxString(Utils::PROPERTY_IDS[PROPERTY_ID_TILING_U_] + std::to_string(i)), selected->Textures[i]->Scale[0], 1.0f, 100.0f, 0.01f);
			this->addPropertyRange(" Tiling V", wxString(Utils::PROPERTY_IDS[PROPERTY_ID_TILING_V_] + std::to_string(i)), selected->Textures[i]->Scale[1], 1.0f, 100.0f, 0.01f);
		}
	}
}

bool WindowFrame::IsPropertiesActive()
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
	if ((index >= 0) && (index < (int)this->listBoxComponents->GetCount())) {
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

	wxString propertyName      = property->GetName();
	wxString propertyNameFirst = propertyName.substr(0, propertyName.size() - 1);
	wxString propertyNameLast  = propertyName.substr(propertyName.size() - 1);

	if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_BOUNDING_VOLUME])
		dynamic_cast<Mesh*>(selected)->SetBoundingVolume(static_cast<BoundingVolumeType>(property->GetChoiceSelection()));
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_ENABLE_AUTO_ROTATION])
		selected->AutoRotate = property->GetValue().GetBool();
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_NAME])
		selected->Name = (!property->GetValueAsString().Trim().empty() ? property->GetValueAsString() : selected->Name);
	// MATERIAL
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_COLOR])
		selected->ComponentMaterial.diffuse = Utils::ToVec4Color(Utils::ToWxColour(property->GetValue()));
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_SPEC_INTENSITY])
		selected->ComponentMaterial.specular.intensity = Utils::ToVec3Color(Utils::ToWxColour(property->GetValue()));
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_SPEC_SHININESS])
		selected->ComponentMaterial.specular.shininess = property->GetValue().GetDouble();
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_OPACITY])
		selected->ComponentMaterial.diffuse.a = (property->GetValue().GetDouble() / 255.0f);
	// HUD
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TRANSPARENCY])
		dynamic_cast<HUD*>(selected->Parent)->Transparent = property->GetValue().GetBool();
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TEXT])
		dynamic_cast<HUD*>(selected->Parent)->Update(property->GetValueAsString());
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_ALIGNMENT])
		dynamic_cast<HUD*>(selected->Parent)->TextAlign = property->GetValueAsString();
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_FONT])
		dynamic_cast<HUD*>(selected->Parent)->TextFont = property->GetValueAsString();
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_SIZE])
		dynamic_cast<HUD*>(selected->Parent)->TextSize = property->GetValue().GetInteger();
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_COLOR])
		dynamic_cast<HUD*>(selected->Parent)->TextColor = Utils::ToWxColour(property->GetValue());
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_HUD_TEXTURE])
		dynamic_cast<Mesh*>(selected)->LoadTextureImage(property->GetValueAsString(), 0);
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_HUD_REMOVE_TEXTURE])
		dynamic_cast<Mesh*>(selected)->RemoveTexture(0);
	// TERRAIN
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_SIZE])
		dynamic_cast<Terrain*>(selected->Parent)->Resize(property->GetValue().GetInteger(), dynamic_cast<Terrain*>(selected->Parent)->Octaves(), dynamic_cast<Terrain*>(selected->Parent)->Redistribution());
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_OCTAVES])
		dynamic_cast<Terrain*>(selected->Parent)->Resize(dynamic_cast<Terrain*>(selected->Parent)->Size(), property->GetValue().GetInteger(), dynamic_cast<Terrain*>(selected->Parent)->Redistribution());
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_REDISTRIBUTION])
		dynamic_cast<Terrain*>(selected->Parent)->Resize(dynamic_cast<Terrain*>(selected->Parent)->Size(), dynamic_cast<Terrain*>(selected->Parent)->Octaves(), property->GetValue().GetDouble());
	// WATER
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_WATER_SPEED])
		dynamic_cast<Water*>(selected->Parent)->FBO()->Speed = property->GetValue().GetDouble();
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_WATER_WAVE_STRENGTH])
		dynamic_cast<Water*>(selected->Parent)->FBO()->WaveStrength = property->GetValue().GetDouble();
	// LIGHT SOURCES
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ACTIVE])
		dynamic_cast<LightSource*>(selected->Parent)->SetActive(property->GetValue().GetBool());
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_LOCATION_])
		dynamic_cast<LightSource*>(selected->Parent)->MoveTo(this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->Position()));
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_AMBIENT])
		dynamic_cast<LightSource*>(selected->Parent)->SetAmbient(Utils::ToVec4Color(Utils::ToWxColour(property->GetValue())));
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_DIFFUSE])
		dynamic_cast<LightSource*>(selected->Parent)->SetColor(Utils::ToVec4Color(Utils::ToWxColour(property->GetValue())));
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_SPEC_INTENSITY])
		dynamic_cast<LightSource*>(selected->Parent)->SetSpecularIntensity(Utils::ToVec4Color(Utils::ToWxColour(property->GetValue())));
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_SPEC_SHININESS])
		dynamic_cast<LightSource*>(selected->Parent)->SetSpecularShininess(property->GetValue().GetDouble());
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_DIRECTION_])
		dynamic_cast<LightSource*>(selected->Parent)->SetDirection(this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), dynamic_cast<LightSource*>(selected->Parent)->Direction()));
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_LINEAR])
		dynamic_cast<LightSource*>(selected->Parent)->SetAttenuationLinear(property->GetValue().GetDouble());
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_QUAD])
		dynamic_cast<LightSource*>(selected->Parent)->SetAttenuationQuadratic(property->GetValue().GetDouble());
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ANGLE_INNER])
		dynamic_cast<LightSource*>(selected->Parent)->SetConeInnerAngle(property->GetValue().GetDouble());
	else if (propertyName == Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ANGLE_OUTER])
		dynamic_cast<LightSource*>(selected->Parent)->SetConeOuterAngle(property->GetValue().GetDouble());
	// VEC3 PROPERTIES
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_AUTO_ROTATION_])
		selected->AutoRotation = this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->AutoRotation);
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_FLIP_TEXTURE_])
		dynamic_cast<Mesh*>(selected)->Textures[std::atoi(propertyNameLast.c_str().AsChar())]->SetFlipY(property->GetValue().GetBool());
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_LOCATION_])
		selected->MoveTo(this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->Position()));
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_REMOVE_TEXTURE_])
		dynamic_cast<Mesh*>(selected)->RemoveTexture(std::atoi(propertyNameLast.c_str().AsChar()));
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_REPEAT_TEXTURE_])
		dynamic_cast<Mesh*>(selected)->Textures[std::atoi(propertyNameLast.c_str().AsChar())]->SetRepeat(property->GetValue().GetBool());
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_ROTATION_])
		selected->RotateTo(this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->Rotation()));
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_TILING_U_])
		dynamic_cast<Mesh*>(selected)->Textures[std::atoi(propertyNameLast.c_str().AsChar())]->Scale[0] = property->GetValue().GetDouble();
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_TILING_V_])
		dynamic_cast<Mesh*>(selected)->Textures[std::atoi(propertyNameLast.c_str().AsChar())]->Scale[1] = property->GetValue().GetDouble();
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_SCALE_])
		selected->ScaleTo(this->updatePropertyXYZ(propertyName, property->GetValue().GetDouble(), selected->Scale()));
	else if (propertyNameFirst == Utils::PROPERTY_IDS[PROPERTY_ID_TEXTURE_])
		dynamic_cast<Mesh*>(selected)->LoadTextureImage(property->GetValueAsString(), std::atoi(propertyNameLast.c_str().AsChar()));

	// HUD
	if (selected->Type() == COMPONENT_HUD)
		dynamic_cast<HUD*>(selected->Parent)->Update();

	return this->UpdateProperties();
}

int WindowFrame::UpdateProperties()
{
	Component* selected = (SceneManager::SelectedChild != nullptr ? SceneManager::SelectedChild : SceneManager::SelectedComponent);

	if ((selected == nullptr) || (this->propertyGrid == nullptr))
		return -1;

	this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_NAME], static_cast<wxString>(selected->Name));

	// LIGHT SOURCES
	if (selected->Type() == COMPONENT_LIGHTSOURCE)
		return this->updatePropertiesLightSources(selected);

	// UPDATE LIST OF COMPONENTS/CHILDREN
	if (selected->Type() == COMPONENT_CAMERA)
		this->listBoxComponents->SetString(0, selected->Name);
	else
		this->SelectComponent(this->listBoxComponents->GetSelection());

	if (selected->Type() != COMPONENT_SKYBOX) {
		glm::vec3 position = selected->Position();
		this->setPropertyXYZ(Utils::PROPERTY_IDS[PROPERTY_ID_LOCATION], position[0], position[1], position[2]);
	}

	if (selected->Type() != COMPONENT_SKYBOX) {
		glm::vec3 rotation = selected->Rotation();
		this->setPropertyXYZ(Utils::PROPERTY_IDS[PROPERTY_ID_ROTATION], rotation[0], rotation[1], rotation[2]);
	}

	if (selected->Type() == COMPONENT_CAMERA)
		return 2;

	if (selected->Type() != COMPONENT_SKYBOX) {
		glm::vec3 scale = selected->Scale();
		this->setPropertyXYZ(Utils::PROPERTY_IDS[PROPERTY_ID_SCALE], scale[0], scale[1], scale[2]);
	}

	if (selected->Type() != COMPONENT_SKYBOX)
	{
		glm::vec3 autoRotation = selected->AutoRotation;

		this->setPropertyXYZ(Utils::PROPERTY_IDS[PROPERTY_ID_AUTO_ROTATION], autoRotation[0], autoRotation[1], autoRotation[2]);
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_ENABLE_AUTO_ROTATION], selected->AutoRotate);
	}

	if (selected->Type() != COMPONENT_SKYBOX) {
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_COLOR],          Utils::ToWxColour(selected->ComponentMaterial.diffuse));
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_SPEC_INTENSITY], Utils::ToWxColour(selected->ComponentMaterial.specular.intensity));
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_SPEC_SHININESS], selected->ComponentMaterial.specular.shininess);
	}

	if (selected->Type() == COMPONENT_HUD)
	{
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_OPACITY],            (selected->ComponentMaterial.diffuse.a * 255.0f));
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TRANSPARENCY],       dynamic_cast<HUD*>(selected->Parent)->Transparent);
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TEXT],               dynamic_cast<HUD*>(selected->Parent)->Text());
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_ALIGNMENT],     dynamic_cast<HUD*>(selected->Parent)->TextAlign);
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_FONT],          dynamic_cast<HUD*>(selected->Parent)->TextFont);
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_SIZE],          dynamic_cast<HUD*>(selected->Parent)->TextSize);
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TEXT_COLOR],         dynamic_cast<HUD*>(selected->Parent)->TextColor);
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_HUD_TEXTURE],        selected->Textures[0]->ImageFile());
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_HUD_REMOVE_TEXTURE], false);
	}
	else
	{
		for (int i = 0; i < MAX_TEXTURES; i++)
		{
			if (selected->Type() == COMPONENT_TERRAIN) {
				if (selected->Textures[i]->ImageFile(0).empty())
					continue;
			} else if (selected->Type() == COMPONENT_WATER) {
				if (selected->Textures[i]->ImageFile(0).empty())
					continue;
			} else {
				if (i > 1)
					continue;
			}

			this->propertyGrid->SetPropertyValue(wxString(Utils::PROPERTY_IDS[PROPERTY_ID_TEXTURE_]        + std::to_string(i)), selected->Textures[i]->ImageFile());
			this->propertyGrid->SetPropertyValue(wxString(Utils::PROPERTY_IDS[PROPERTY_ID_REMOVE_TEXTURE_] + std::to_string(i)), false);

			if ((selected->Type() != COMPONENT_SKYBOX) && (selected->Type() != COMPONENT_TERRAIN) && (selected->Type() != COMPONENT_WATER)) {
				this->propertyGrid->SetPropertyValue(wxString(Utils::PROPERTY_IDS[PROPERTY_ID_FLIP_TEXTURE_]   + std::to_string(i)), selected->Textures[i]->FlipY());
				this->propertyGrid->SetPropertyValue(wxString(Utils::PROPERTY_IDS[PROPERTY_ID_REPEAT_TEXTURE_] + std::to_string(i)), selected->Textures[i]->Repeat());
			}

			this->propertyGrid->SetPropertyValue(wxString(Utils::PROPERTY_IDS[PROPERTY_ID_TILING_U_] + std::to_string(i)), selected->Textures[i]->Scale[0]);
			this->propertyGrid->SetPropertyValue(wxString(Utils::PROPERTY_IDS[PROPERTY_ID_TILING_V_] + std::to_string(i)), selected->Textures[i]->Scale[1]);
		}
	}

	if ((selected->Type() == COMPONENT_HUD) || (selected->Type() == COMPONENT_SKYBOX))
		return 3;

	if ((selected->Type() != COMPONENT_TERRAIN) && (selected->Type() != COMPONENT_WATER))
	{
		BoundingVolume*    volume     = dynamic_cast<Mesh*>(selected)->GetBoundingVolume();
		BoundingVolumeType volumeType = (volume != nullptr ? volume->VolumeType() : BOUNDING_VOLUME_NONE);

		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_BOUNDING_VOLUME], volumeType);
	}

	if (selected->Type() == COMPONENT_TERRAIN)
	{
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_SIZE],           dynamic_cast<Terrain*>(selected->Parent)->Size());
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_OCTAVES],        dynamic_cast<Terrain*>(selected->Parent)->Octaves());
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_TERRAIN_REDISTRIBUTION], dynamic_cast<Terrain*>(selected->Parent)->Redistribution());
	}

	if (selected->Type() == COMPONENT_WATER)
	{
		WaterFBO* fbo = dynamic_cast<Water*>(selected->Parent)->FBO();

		if (fbo != nullptr) {
			this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_WATER_SPEED],         fbo->Speed);
			this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_WATER_WAVE_STRENGTH], fbo->WaveStrength);
		}
	}

	return 0;
}

int WindowFrame::updatePropertiesLightSources(Component* selected)
{
	Attenuation  attenuation;
	glm::vec3    direction;
	LightSource* lightSource = dynamic_cast<LightSource*>(selected->Parent);

	// TOGGLE ACTIVE STATE
	this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ACTIVE], lightSource->Active());

	// LOCATION / POSITION
	glm::vec3 position = selected->Position();
	this->setPropertyXYZ(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_LOCATION], position[0], position[1], position[2]);

	// MATERIAL
	Material material = lightSource->GetMaterial();

	this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_AMBIENT],        Utils::ToWxColour(material.ambient));
	this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_DIFFUSE],        Utils::ToWxColour(material.diffuse));
	this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_SPEC_INTENSITY], Utils::ToWxColour(material.specular.intensity));
	this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_SPEC_SHININESS], material.specular.shininess);

	switch (lightSource->SourceType()) {
	case ID_ICON_LIGHT_DIRECTIONAL:
		direction = lightSource->Direction();
		this->setPropertyXYZ(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_DIRECTION], direction[0], direction[1], direction[2]);

		break;
	case ID_ICON_LIGHT_POINT:
		attenuation = lightSource->GetAttenuation();

		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_LINEAR], attenuation.linear);
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_QUAD],   attenuation.quadratic);

		break;
	case ID_ICON_LIGHT_SPOT:
		direction = lightSource->Direction();
		this->setPropertyXYZ(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_DIRECTION], direction[0], direction[1], direction[2]);
		
		attenuation = lightSource->GetAttenuation();

		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_LINEAR], attenuation.linear);
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ATT_QUAD],   attenuation.quadratic);

		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ANGLE_INNER], lightSource->GetConeInnerAngle());
		this->propertyGrid->SetPropertyValue(Utils::PROPERTY_IDS[PROPERTY_ID_LIGHT_ANGLE_OUTER], lightSource->GetConeOuterAngle());

		break;
	default:
		throw;
	}

	return 0;
}
