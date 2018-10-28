#ifndef GE3D_GLOBALS_H
#include "../globals.h"
#endif

#ifndef GE3D_SCENEMANAGER_H
#define GE3D_SCENEMANAGER_H

class SceneManager
{
public:
	static glm::vec3 AmbientLightIntensity;
	static glm::vec4 SelectColor;
	static Light     SunLight;

	static std::vector<Component*> Components;
	static Texture*                EmptyCubemap;
	static Texture*                EmptyTexture;
	static Mesh*                   SelectedChild;
	static Component*              SelectedComponent;

private:
	SceneManager()  {}
	~SceneManager() {}

public:
	static int      AddComponent(Component* component);
	static void     Clear();
	static int      GetComponentIndex(Component* component);
	static HUD*     LoadHUD();
	static Model*   LoadModel(const wxString &file);
	static int      LoadScene(const wxString &file);
	static Skybox*  LoadSkybox();
	static Terrain* LoadTerrain(int size, float octaves, float redistribution);
	static Water*   LoadWater();
	static void     RemoveSelectedComponent();
	static void     RemoveSelectedChild();
	static int      SaveScene(const wxString &file);
	static void     SelectComponent(int index);
	static void     SelectChild(int index);

};

#endif
