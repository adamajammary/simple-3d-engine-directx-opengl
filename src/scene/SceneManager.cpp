#include "SceneManager.h"

std::vector<Component*> SceneManager::Components;
Texture*                SceneManager::EmptyCubemap      = nullptr;
Texture*                SceneManager::EmptyTexture      = nullptr;
bool                    SceneManager::Ready             = true;
Component*              SceneManager::SelectedChild     = nullptr;
Component*              SceneManager::SelectedComponent = nullptr;
glm::vec4               SceneManager::SelectColor       = { 1.0f, 0.5f, 0.0f, 1.0f };

LightSource* SceneManager::LightSources[MAX_LIGHT_SOURCES] = {};

int SceneManager::AddComponent(Component* component)
{
	if (component == nullptr)
		return -1;

	switch (component->Type()) {
	case COMPONENT_CAMERA:
        RenderEngine::CameraMain = (Camera*)component;
		break;
	case COMPONENT_HUD:
        for (auto child : component->Children)
			RenderEngine::HUDs.push_back(child);
		break;
	case COMPONENT_SKYBOX:
		if (!component->Children.empty())
			RenderEngine::Skybox = dynamic_cast<Mesh*>(component->Children[0]);
		break;
	case COMPONENT_TERRAIN:
		for (auto child : component->Children)
			RenderEngine::Terrains.push_back(child);
		break;
	case COMPONENT_WATER:
		for (auto child : component->Children)
			RenderEngine::Waters.push_back(child);
		break;
	case COMPONENT_LIGHTSOURCE:
		if (SceneManager::AddLightSource(component) < 0)
			return -2;

		if (dynamic_cast<LightSource*>(component)->SourceType() != ID_ICON_LIGHT_DIRECTIONAL) {
			for (auto child : component->Children)
				RenderEngine::LightSources.push_back(child);
		}

		break;
	default:
		for (auto child : component->Children)
			RenderEngine::Renderables.push_back(child);
		break;
	}

	SceneManager::SelectedComponent = component;
	SceneManager::Components.push_back(component);
	RenderEngine::Canvas.Window->AddListComponent(component);
	RenderEngine::Canvas.Window->AddListChildren(SceneManager::SelectedComponent->Children);

	if (SceneManager::Ready)
		RenderEngine::Canvas.Window->InitProperties();

	return 0;
}

int SceneManager::AddLightSource(Component* component)
{
	LightSource* lightSource = dynamic_cast<LightSource*>(component);

	if (lightSource == nullptr)
		return -1;

	for (uint32_t i = 0; i < MAX_LIGHT_SOURCES; i++) {
		if (SceneManager::LightSources[i] == nullptr) {
			SceneManager::LightSources[i] = lightSource;
			return 0;
		}
	}

	wxMessageBox(("WARNING: Max " + std::to_wstring(MAX_LIGHT_SOURCES) + " light sources are supported in the scene."), RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);

	return -2;
}

void SceneManager::Clear()
{
	RenderEngine::CameraMain            = nullptr;
	SceneManager::SelectedComponent = nullptr;
	SceneManager::SelectedChild     = nullptr;
	RenderEngine::Skybox            = nullptr;

	for (uint32_t i = 0; i < MAX_LIGHT_SOURCES; i++)
		SceneManager::LightSources[i] = nullptr;

	RenderEngine::HUDs.clear();
    RenderEngine::Terrains.clear();
    RenderEngine::Waters.clear();
	RenderEngine::LightSources.clear();
	RenderEngine::Renderables.clear();

	for (auto it = SceneManager::Components.begin(); it != SceneManager::Components.end(); it++)
		_DELETEP(*it);

	SceneManager::Components.clear();

	if (RenderEngine::Canvas.Window != nullptr)
		RenderEngine::Canvas.Window->ClearScene();
}

int SceneManager::GetComponentIndex(Component* component)
{
	for (int i = 0; i < (int)SceneManager::Components.size(); i++) {
		if (SceneManager::Components[i] == component)
			return i;
	}

	return -1;
}

HUD* SceneManager::LoadHUD()
{
	HUD* hud = new HUD(Utils::RESOURCE_MODELS[ID_ICON_QUAD]);

	if ((hud == nullptr) || !hud->IsValid())
		return nullptr;

	SceneManager::AddComponent(hud);

	return hud;
}

LightSource* SceneManager::LoadLightSource(IconType type)
{
	LightSource* light = new LightSource(Utils::RESOURCE_MODELS[ID_ICON_ICO_SPHERE], type);

	if ((light == nullptr) || !light->IsValid())
		return nullptr;

	if (SceneManager::AddComponent(light) < 0)
		_DELETEP(light);

	return light;
}

Model* SceneManager::LoadModel(const wxString &file)
{
	if (file.empty())
		return nullptr;

	Model* model = new Model(file);

	if ((model == nullptr) || !model->IsValid())
		return nullptr;

	SceneManager::AddComponent(model);

	return model;
}

int SceneManager::LoadScene(const wxString &file)
{
	SceneManager::Ready = false;

	if (file.empty())
		return -1;

	std::vector<uint8_t> sceneFile   = Utils::LoadDataFile(file);
	std::vector<uint8_t> sceneBuffer = Utils::Decompress(sceneFile);
	std::string          sceneData   = Utils::ToString(sceneBuffer);

	if (sceneData.empty()) {
		RenderEngine::Canvas.Window->SetStatusText("Failed to load the scene '" + file + "'");
		return -1;
	}

	SceneManager::Clear();

	if (RenderEngine::CameraMain == nullptr)
		SceneManager::AddComponent(new Camera());

	std::string error;
	auto        sceneDataJSON = json11::Json::parse(sceneData, error);
	Component*  component     = nullptr;

	// COMPONENTS
	for (auto &componentJSON : sceneDataJSON["components"].array_items())
	{
		ComponentType type = (ComponentType)componentJSON["type"].int_value();

		switch (type) {
		case COMPONENT_CAMERA:
			RenderEngine::CameraMain->Name = componentJSON["name"].string_value();
			RenderEngine::CameraMain->MoveTo(Utils::ToVec3(componentJSON["position"].array_items()));
			RenderEngine::CameraMain->RotateTo(Utils::ToVec3(componentJSON["rotation"].array_items()));

			break;
		case COMPONENT_HUD:
			component       = SceneManager::LoadHUD();
			component->Name = componentJSON["name"].string_value();

			dynamic_cast<HUD*>(component)->Transparent = componentJSON["transparent"].bool_value();
			dynamic_cast<HUD*>(component)->TextAlign   = componentJSON["text_align"].string_value();
			dynamic_cast<HUD*>(component)->TextFont    = componentJSON["text_font"].string_value();
			dynamic_cast<HUD*>(component)->TextSize    = componentJSON["text_size"].int_value();
			dynamic_cast<HUD*>(component)->TextColor   = Utils::ToWxColour(Utils::ToVec4(componentJSON["text_color"].array_items()));

			dynamic_cast<HUD*>(component)->Update(componentJSON["text"].string_value());

			break;
		case COMPONENT_MODEL:
			component       =  SceneManager::LoadModel(componentJSON["model_file"].string_value());
			component->Name = componentJSON["name"].string_value();

			break;
		case COMPONENT_SKYBOX:
			component       =  SceneManager::LoadSkybox();
			component->Name = componentJSON["name"].string_value();
			
			break;
		case COMPONENT_TERRAIN:
			component       =  SceneManager::LoadTerrain(componentJSON["size"].int_value(), componentJSON["octaves"].int_value(), componentJSON["redistribution"].number_value());
			component->Name = componentJSON["name"].string_value();
			
			break;
		case COMPONENT_WATER:
			component       = SceneManager::LoadWater();
			component->Name = componentJSON["name"].string_value();

			dynamic_cast<Water*>(component)->FBO()->Speed        = componentJSON["speed"].number_value();
			dynamic_cast<Water*>(component)->FBO()->WaveStrength = componentJSON["wave_strength"].number_value();
			
			break;
		case COMPONENT_LIGHTSOURCE:
			component       = SceneManager::LoadLightSource((IconType)componentJSON["source_type"].int_value());
			component->Name = componentJSON["name"].string_value();

			dynamic_cast<LightSource*>(component)->SetActive(componentJSON["active"].bool_value());
			dynamic_cast<LightSource*>(component)->SetAmbient(Utils::ToVec3(componentJSON["ambient"].array_items()));
			dynamic_cast<LightSource*>(component)->SetColor(Utils::ToVec4(componentJSON["diffuse"].array_items()));
			dynamic_cast<LightSource*>(component)->SetSpecularIntensity(Utils::ToVec3(componentJSON["spec_intensity"].array_items()));
			dynamic_cast<LightSource*>(component)->SetSpecularShininess(componentJSON["spec_shininess"].number_value());
			dynamic_cast<LightSource*>(component)->SetDirection(Utils::ToVec3(componentJSON["direction"].array_items()));
			dynamic_cast<LightSource*>(component)->SetAttenuationLinear(componentJSON["att_linear"].number_value());
			dynamic_cast<LightSource*>(component)->SetAttenuationQuadratic(componentJSON["att_quadratic"].number_value());
			dynamic_cast<LightSource*>(component)->SetConeInnerAngle(componentJSON["inner_angle"].number_value());
			dynamic_cast<LightSource*>(component)->SetConeOuterAngle(componentJSON["outer_angle"].number_value());

			break;
		default:
			throw;
		}

		// CHILDREN
		auto childrenJSON = componentJSON["children"].array_items();

		for (int i = 0; i < (int)childrenJSON.size(); i++)
		{
			auto  childJSON = childrenJSON[i];
			Mesh* child     = dynamic_cast<Mesh*>(component->Children[i]);
			
			if ((childJSON == nullptr) || (child == nullptr))
				continue;

			child->Name = childJSON["name"].string_value();

			glm::vec3 position = Utils::ToVec3(childJSON["position"].array_items());
			glm::vec3 scale    = Utils::ToVec3(childJSON["scale"].array_items());

			if (child->Type() == COMPONENT_HUD)
			{
				// Invert Y-axis for both mesh and vertex positions on Vulkan
				if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN) {
					position.y *= -1;
					scale.y    *= -1;
				}
			}

			child->MoveTo(position);
			child->ScaleTo(scale);
			child->RotateTo(Utils::ToVec3(childJSON["rotation"].array_items()));

			child->AutoRotation = Utils::ToVec3(childJSON["auto_rotation"].array_items());
			child->AutoRotate   = childJSON["auto_rotate"].bool_value();
			child->ComponentMaterial.diffuse            = Utils::ToVec4(childJSON["color"].array_items());
			child->ComponentMaterial.specular.intensity = Utils::ToVec3(childJSON["spec_intensity"].array_items());
			child->ComponentMaterial.specular.shininess = childJSON["spec_shininess"].number_value();

			if (child->Type() == COMPONENT_HUD)
				dynamic_cast<HUD*>(child->Parent)->Update();

			child->SetBoundingVolume(static_cast<BoundingVolumeType>(childJSON["bounding_box"].int_value()));

			// TEXTURES
			auto texturesJSON = childJSON["textures"].array_items();

			for (int j = 0; j < (int)texturesJSON.size(); j++)
			{
				auto     textureJSON = texturesJSON[j];
				Texture* texture     = ((type == COMPONENT_WATER) ? dynamic_cast<Water*>(component)->FBO()->Textures[j] : child->Textures[j]);

				if ((textureJSON == nullptr) || (texture == nullptr))
					continue;

				// TERRAIN, WATER
				if ((type == COMPONENT_TERRAIN) || (type == COMPONENT_WATER)) {
					texture->Scale = Utils::ToVec2(textureJSON["scale"].array_items());
				// SKYBOX
				} else if (type == COMPONENT_SKYBOX) {
					//
				}
				// MODEL
				else
				{
					// HUD
					if ((type == COMPONENT_HUD) && (j > 0))
						continue;

					wxString imageFile = textureJSON["image_file"].string_value();

					if (imageFile.empty())
						continue;

					texture = new Texture(
						imageFile,
						textureJSON["srgb"].bool_value(),
						textureJSON["repeat"].bool_value(),
						textureJSON["flip"].bool_value(),
						textureJSON["transparent"].bool_value(),
						Utils::ToVec2(textureJSON["scale"].array_items())
					);

					child->LoadTexture(texture, j);
				}
			}
		}

		if ((component != nullptr) && (component->Type() == COMPONENT_LIGHTSOURCE))
			dynamic_cast<LightSource*>(component)->MoveTo(Utils::ToVec3(componentJSON["position"].array_items()));
	}

	RenderEngine::Canvas.Window->InitProperties();
	RenderEngine::Canvas.Window->SetStatusText("Finished loading the scene '" + file + "'");

	SceneManager::Ready = true;

	return 0;
}

Skybox* SceneManager::LoadSkybox()
{
	std::vector<wxString> imageFiles;

	imageFiles.push_back(Utils::RESOURCE_IMAGES["skyboxRight"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["skyboxLeft"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["skyboxTop"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["skyboxBottom"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["skyboxBack"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["skyboxFront"]);

	Skybox* skybox = new Skybox(Utils::RESOURCE_MODELS[ID_ICON_CUBE], imageFiles);

    if ((skybox == nullptr) || !skybox->IsValid())
		return nullptr;

	if (RenderEngine::Skybox != nullptr)
	{
		int index = SceneManager::GetComponentIndex(RenderEngine::Skybox);

		SceneManager::Components.erase(SceneManager::Components.begin() + index);
		RenderEngine::Canvas.Window->RemoveComponent(index);
	}

	SceneManager::AddComponent(skybox);

    return skybox;
}

Terrain* SceneManager::LoadTerrain(int size, float octaves, float redistribution)
{
	std::vector<wxString> imageFiles;

	imageFiles.push_back(Utils::RESOURCE_IMAGES["backgroundTexture"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["rTexture"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["gTexture"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["bTexture"]);
	imageFiles.push_back(Utils::RESOURCE_IMAGES["blendMap"]);

	Terrain* terrain = new Terrain(imageFiles, size, octaves, redistribution);

	if ((terrain == nullptr) || !terrain->IsValid())
		return nullptr;

	SceneManager::AddComponent(terrain);

    return terrain;
}

Water* SceneManager::LoadWater()
{
	std::vector<wxString> imageFiles = { Utils::RESOURCE_IMAGES["duDvMap"], Utils::RESOURCE_IMAGES["normalMap"] };
	Water*                water      = new Water(Utils::RESOURCE_MODELS[ID_ICON_PLANE], imageFiles);

	if ((water == nullptr) || !water->IsValid())
		return nullptr;

	SceneManager::AddComponent(water);

	return water;
}

int SceneManager::RemoveSelectedComponent()
{
	if ((SceneManager::SelectedComponent == nullptr) || (SceneManager::SelectedComponent->Type() == COMPONENT_CAMERA))
		return -1;

	for (auto child : SceneManager::SelectedComponent->Children)
		RenderEngine::RemoveMesh(child);

	if (SceneManager::SelectedComponent->Type() == COMPONENT_LIGHTSOURCE)
		SceneManager::removeSelectedLightSource();

	int index = SceneManager::GetComponentIndex(SceneManager::SelectedComponent);

	SceneManager::SelectComponent(index - 1);
	SceneManager::Components.erase(SceneManager::Components.begin() + index);
	RenderEngine::Canvas.Window->RemoveComponent(index);

	return 0;
}

int SceneManager::RemoveSelectedChild()
{
	if (SceneManager::SelectedChild == nullptr)
		return -1;

	RenderEngine::RemoveMesh(SceneManager::SelectedChild);

	int index = SceneManager::SelectedComponent->GetChildIndex(SceneManager::SelectedChild);

	SceneManager::SelectedComponent->Children.erase(SceneManager::SelectedComponent->Children.begin() + index);
	RenderEngine::Canvas.Window->RemoveChild(index);

	if (!SceneManager::SelectedComponent->Children.empty())
	{
		if (index == (int)SceneManager::SelectedComponent->Children.size())
			SceneManager::SelectChild(index - 1);
		else
			SceneManager::SelectChild(index);
	} else {
		SceneManager::RemoveSelectedComponent();
	}

	return 0;
}

void SceneManager::removeSelectedLightSource()
{
	for (uint32_t i = 0; i < MAX_LIGHT_SOURCES; i++) {
		if (SceneManager::SelectedComponent == SceneManager::LightSources[i]) {
			_DELETEP(SceneManager::LightSources[i]);
			break;
		}
	}
}

int SceneManager::SaveScene(const wxString &file)
{
	if (file.empty())
		return -1;

	RenderEngine::Canvas.Window->SetStatusText("Saving the scene to file '" + file + "'");

	json11::Json::array componentsJSON;

	for (auto component : SceneManager::Components)
	{
		json11::Json::array childrenJSON;

		// CHILDREN
		for (auto child : component->Children)
		{
			if (child->Type() == COMPONENT_CAMERA)
				continue;

			json11::Json        childJSON;
			json11::Json::array texturesJSON;

			for (int i = 0; i < MAX_TEXTURES; i++)
			{
				json11::Json textureJSON;
				Texture*     texture;

				if (component->Type() == COMPONENT_WATER)
					texture = dynamic_cast<Water*>(component)->FBO()->Textures[i];
				else
					texture = child->Textures[i];

				textureJSON = json11::Json::object {
					{ "image_file",  static_cast<std::string>(texture->ImageFile()) },
					{ "srgb",        texture->SRGB() },
					{ "repeat",      texture->Repeat() },
					{ "flip",        texture->FlipY() },
					{ "transparent", texture->Transparent() },
					{ "scale",       Utils::ToJsonArray(texture->Scale) }
				};

				texturesJSON.push_back(textureJSON);
			}

			BoundingVolume* boundingVolume = dynamic_cast<Mesh*>(child)->GetBoundingVolume();
			glm::vec3       position       = child->Position();
			glm::vec3       scale          = child->Scale();

			if (child->Type() == COMPONENT_HUD)
			{
				// Invert Y-axis for both mesh and vertex positions on Vulkan
				if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN) {
					position.y *= -1;
					scale.y    *= -1;
				}
			}

			childJSON = json11::Json::object {
				{ "name",            static_cast<std::string>(child->Name) },
				{ "position",        Utils::ToJsonArray(position) },
				{ "scale",           Utils::ToJsonArray(scale) },
				{ "rotation",        Utils::ToJsonArray(child->Rotation()) },
				{ "auto_rotation",   Utils::ToJsonArray(child->AutoRotation) },
				{ "auto_rotate",     child->AutoRotate },
				{ "color",           Utils::ToJsonArray(child->ComponentMaterial.diffuse) },
				{ "spec_intensity",  Utils::ToJsonArray(child->ComponentMaterial.specular.intensity) },
				{ "spec_shininess",  child->ComponentMaterial.specular.shininess },
				{ "bounding_box",    (boundingVolume != nullptr ? boundingVolume->VolumeType() : BOUNDING_VOLUME_NONE) },
				{ "textures",        texturesJSON }
			};
                
			childrenJSON.push_back(childJSON);
		}

		json11::Json componentJSON;

		switch (component->Type()) {
		case COMPONENT_CAMERA:
			componentJSON = json11::Json::object {
				{ "type",           (int)component->Type() },
				{ "name",           static_cast<std::string>(component->Name) },
				{ "position",       Utils::ToJsonArray(component->Position()) },
				{ "rotation",       Utils::ToJsonArray(component->Rotation()) },
				{ "children",       childrenJSON }
			};
			break;
		case COMPONENT_HUD:
			componentJSON = json11::Json::object {
				{ "type",           (int)component->Type() },
				{ "transparent",    dynamic_cast<HUD*>(component)->Transparent },
				{ "text",           static_cast<std::string>(dynamic_cast<HUD*>(component)->Text()) },
				{ "text_align",     static_cast<std::string>(dynamic_cast<HUD*>(component)->TextAlign) },
				{ "text_font",      static_cast<std::string>(dynamic_cast<HUD*>(component)->TextFont) },
				{ "text_size",      dynamic_cast<HUD*>(component)->TextSize },
				{ "text_color",     Utils::ToJsonArray(Utils::ToVec4Color(dynamic_cast<HUD*>(component)->TextColor)) },
				{ "children",       childrenJSON }
			};
			break;
		case COMPONENT_MODEL:
			componentJSON = json11::Json::object {
				{ "type",           (int)component->Type() },
				{ "name",           static_cast<std::string>(component->Name) },
				{ "model_file",     static_cast<std::string>(component->ModelFile()) },
				{ "children",       childrenJSON }
			};
			break;
		case COMPONENT_SKYBOX:
			componentJSON = json11::Json::object {
				{ "type",           (int)component->Type() },
				{ "name",           static_cast<std::string>(component->Name) },
				{ "children",       childrenJSON }
			};
			break;
		case COMPONENT_TERRAIN:
			componentJSON = json11::Json::object {
				{ "type",           (int)component->Type() },
				{ "name",           static_cast<std::string>(component->Name) },
				{ "size",           dynamic_cast<Terrain*>(component)->Size() },
				{ "octaves",        dynamic_cast<Terrain*>(component)->Octaves() },
				{ "redistribution", dynamic_cast<Terrain*>(component)->Redistribution() },
				{ "children",       childrenJSON }
			};
			break;
		case COMPONENT_WATER:
			componentJSON = json11::Json::object {
				{ "type",           (int)component->Type() },
				{ "name",           static_cast<std::string>(component->Name) },
				{ "speed",          dynamic_cast<Water*>(component)->FBO()->Speed },
				{ "wave_strength",  dynamic_cast<Water*>(component)->FBO()->WaveStrength },
				{ "children",       childrenJSON }
			};
			break;
		case COMPONENT_LIGHTSOURCE:
			componentJSON = json11::Json::object{
				{ "type",           (int)component->Type() },
				{ "name",           static_cast<std::string>(component->Name) },
				{ "source_type",    (int)dynamic_cast<LightSource*>(component)->SourceType() },
				{ "active",         dynamic_cast<LightSource*>(component)->Active() },
				{ "position",       Utils::ToJsonArray(component->Children[0]->Position()) },
				{ "ambient",        Utils::ToJsonArray(dynamic_cast<LightSource*>(component)->GetMaterial().ambient) },
				{ "diffuse",        Utils::ToJsonArray(dynamic_cast<LightSource*>(component)->GetMaterial().diffuse) },
				{ "spec_intensity", Utils::ToJsonArray(dynamic_cast<LightSource*>(component)->GetMaterial().specular.intensity) },
				{ "spec_shininess", dynamic_cast<LightSource*>(component)->GetMaterial().specular.shininess },
				{ "direction",      Utils::ToJsonArray(dynamic_cast<LightSource*>(component)->Direction()) },
				{ "att_linear",     dynamic_cast<LightSource*>(component)->GetAttenuation().linear },
				{ "att_quadratic",  dynamic_cast<LightSource*>(component)->GetAttenuation().quadratic },
				{ "inner_angle",    dynamic_cast<LightSource*>(component)->ConeInnerAngle() },
				{ "outer_angle",    dynamic_cast<LightSource*>(component)->ConeOuterAngle() },
				{ "children",       childrenJSON }
			};

			break;
		default:
			throw;
		}

		componentsJSON.push_back(componentJSON);
	}

	json11::Json sceneDataJSON = json11::Json::object {
		{ "components", componentsJSON }
	};

	std::string          sceneData = sceneDataJSON.dump();
	std::vector<uint8_t> outBuffer = Utils::Compress(std::vector<uint8_t>(sceneData.c_str(), sceneData.c_str() + sceneData.size()));
	int                  result    = Utils::SaveDataToFile(outBuffer, file, sceneData.size());

	RenderEngine::Canvas.Window->SetStatusText("Finished saving the scene to file '" + file + "'");

	return result;
}

int SceneManager::SelectComponent(int index)
{
	if ((index < 0) || (index >= (int)SceneManager::Components.size()))
		return -1;

	SceneManager::SelectedChild     = nullptr;
	SceneManager::SelectedComponent = SceneManager::Components[index];
	RenderEngine::Canvas.Window->SelectComponent(index);
	RenderEngine::Canvas.Window->InitProperties();

	return 0;
}

int SceneManager::SelectChild(int index)
{
	if ((SceneManager::SelectedComponent == nullptr) ||
		(index < 0) || (index >= (int)SceneManager::SelectedComponent->Children.size()))
	{
		return -1;
	}

	SceneManager::SelectedChild = SceneManager::SelectedComponent->Children[index];
	RenderEngine::Canvas.Window->SelectChild(index);
	RenderEngine::Canvas.Window->InitProperties();

	return 0;
}
