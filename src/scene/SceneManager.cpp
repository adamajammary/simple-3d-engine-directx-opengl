#include "SceneManager.h"

std::vector<Component*> SceneManager::Components;
Texture*                SceneManager::EmptyCubemap      = nullptr;
Texture*                SceneManager::EmptyTexture      = nullptr;
Component*              SceneManager::SelectedChild     = nullptr;
Component*              SceneManager::SelectedComponent = nullptr;
glm::vec4               SceneManager::SelectColor       = { 1.0f, 0.5f, 0.0f, 1.0f };

DirectionalLight SceneManager::SunLight = DirectionalLight(
	{ -0.1f, -0.5f, -1.0f },				// DIRECTION
	Light(
		{ 10.0f, 50.0f, 100.0f },			// POSITION
		Material(
			{ 0.4f, 0.4f, 0.4f, 1.0f },		// DIFFUSE (COLOR)
			{ 0.2f, 0.2f, 0.2f },			// AMBIENT
			Specular(
				{ 0.6f, 0.6f, 0.6f },		// INTENSITY
				20.0f						// SHININESS
			)
		)
	)
);

int SceneManager::AddComponent(Component* component)
{
	if (component == nullptr)
		return -1;

	switch (component->Type()) {
	case COMPONENT_CAMERA:
        RenderEngine::Camera = (Camera*)component;
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
	default:
		for (auto child : component->Children)
			RenderEngine::Renderables.push_back(child);
		break;
	}

	SceneManager::SelectedComponent = component;
	SceneManager::Components.push_back(component);
	RenderEngine::Canvas.Window->AddListComponent(component);
	RenderEngine::Canvas.Window->AddListChildren(SceneManager::SelectedComponent->Children);
	RenderEngine::Canvas.Window->InitProperties();

	return 0;
}

void SceneManager::Clear()
{
	RenderEngine::Skybox = nullptr;

	RenderEngine::HUDs.clear();
    RenderEngine::Terrains.clear();
    RenderEngine::Waters.clear();
    RenderEngine::Renderables.clear();

	if (SceneManager::Components.size() > 1) {
		for (auto it = SceneManager::Components.begin() + 1; it != SceneManager::Components.end(); it++)
			_DELETEP(*it);
	}

	SceneManager::SelectedComponent = nullptr;
	SceneManager::SelectedChild     = nullptr;

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

	std::string error;
	auto        sceneDataJSON = json11::Json::parse(sceneData, error);
	Component*  component     = nullptr;

	// COMPONENTS
	for (auto &componentJSON : sceneDataJSON["components"].array_items())
	{
		ComponentType type = (ComponentType)componentJSON["type"].int_value();

		switch (type) {
		case COMPONENT_CAMERA:
			RenderEngine::Camera->Name = componentJSON["name"].string_value();
			RenderEngine::Camera->MoveTo(Utils::ToVec3(componentJSON["position"].array_items()));
			RenderEngine::Camera->RotateTo(Utils::ToVec3(componentJSON["rotation"].array_items()));

			break;
		case COMPONENT_HUD:
			component                                  = SceneManager::LoadHUD();
			component->Name                            = componentJSON["name"].string_value();
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
			component                                            = SceneManager::LoadWater();
			component->Name                                      = componentJSON["name"].string_value();
			dynamic_cast<Water*>(component)->FBO()->Speed        = componentJSON["speed"].number_value();
			dynamic_cast<Water*>(component)->FBO()->WaveStrength = componentJSON["wave_strength"].number_value();
			
			break;
		//case COMPONENT_MATERIAL:
		//	break;
		//case COMPONENT_LIGHT:
		//	break;
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
						textureJSON["repeat"].bool_value(),
						textureJSON["flip"].bool_value(),
						textureJSON["transparent"].bool_value(),
						Utils::ToVec2(textureJSON["scale"].array_items())
					);

					child->LoadTexture(texture, j);
				}
			}
		}
	}

	RenderEngine::Canvas.Window->UpdateProperties();
	RenderEngine::Canvas.Window->SetStatusText("Finished loading the scene '" + file + "'");

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
		//case COMPONENT_LIGHT:
		//	break;
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
