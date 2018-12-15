#ifndef GE3D_GLOBALS_H
#include "../globals.h"
#endif

#ifndef GE3D_SCENEMANAGER_H
#define GE3D_SCENEMANAGER_H

class SceneManager
{
public:
	static glm::vec4    SelectColor;
	static LightSource* LightSources[];

	static std::vector<Component*> Components;
	static Texture*                EmptyCubemap;
	static Texture*                EmptyTexture;
	static bool                    Ready;
	static Component*              SelectedChild;
	static Component*              SelectedComponent;

private:
	SceneManager()  {}
	~SceneManager() {}

public:
	static int          AddComponent(Component* component);
	static int          AddLightSource(Component* component);
	static void         Clear();
	static int          GetComponentIndex(Component* component);
	static HUD*         LoadHUD();
	static LightSource* LoadLightSource(IconType type);
	static Model*       LoadModel(const wxString &file);
	static int          LoadScene(const wxString &file);
	static Skybox*      LoadSkybox();
	static Terrain*     LoadTerrain(int size = 10, float octaves = 1.0f, float redistribution = 2.0f);
	static Water*       LoadWater();
	static int          RemoveSelectedComponent();
	static int          RemoveSelectedChild();
	static int          SaveScene(const wxString &file);
	static int          SelectComponent(int index);
	static int          SelectChild(int index);

private:
	static void removeSelectedLightSource();

};

#endif
