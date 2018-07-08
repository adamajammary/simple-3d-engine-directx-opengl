#include "ShaderManager.h"

ShaderProgram* ShaderManager::Programs[NR_OF_SHADERS];

void ShaderManager::Close()
{
	for (int i = 0; i < NR_OF_SHADERS; i++)
		_DELETEP(ShaderManager::Programs[i]);
}

int ShaderManager::Init()
{
	ShaderManager::Close();

	//for (int i = 0; i < NR_OF_SHADERS; i++)
	//	_DELETEP(ShaderManager::Programs[i]);

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		for (int i = 0; i < NR_OF_SHADERS; i++)
		{
			ShaderManager::Programs[i] = new ShaderProgram(Utils::SHADER_RESOURCES_DX[i].Name);
			int result                 = ShaderManager::Programs[i]->Load(Utils::SHADER_RESOURCES_DX[i].File);

			if ((result != 0) || !ShaderManager::Programs[i]->IsOK())
				return -1;
		}

		break;
	#endif
	case GRAPHICS_API_OPENGL:
		if ((int)Utils::SHADER_RESOURCES_GL.size() < NR_OF_SHADERS * 2)
			return -1;

		for (int i = 0; i < NR_OF_SHADERS; i++)
		{
			Resource vs = Utils::SHADER_RESOURCES_GL[(i * 2) + 0];
			Resource fs = Utils::SHADER_RESOURCES_GL[(i * 2) + 1];

			if (vs.Name.rfind("_vs") == wxString::npos)
				continue;
		
			vs.Result = Utils::LoadTextFile(vs.File);
			fs.Result = Utils::LoadTextFile(fs.File);

			ShaderManager::Programs[i] = new ShaderProgram(vs.Name.substr(0, vs.Name.rfind("_")));
			int result                 = ShaderManager::Programs[i]->LoadAndLink(vs.Result, fs.Result);

			if ((result != 0) || !ShaderManager::Programs[i]->IsOK()) {
				ShaderManager::Programs[i]->Log();
				return -1;
			}
		}

		break;
	case GRAPHICS_API_VULKAN:
		if ((int)Utils::SHADER_RESOURCES_VULKAN.size() < 1 * 2)
			return -1;

		for (int i = 0; i < 1; i++)
		{
			Resource vs = Utils::SHADER_RESOURCES_VULKAN[(i * 2) + 0];
			Resource fs = Utils::SHADER_RESOURCES_VULKAN[(i * 2) + 1];

			if (vs.Name.rfind("_vs") == wxString::npos)
				continue;

			ShaderManager::Programs[i] = new ShaderProgram(vs.Name.substr(0, vs.Name.rfind("_")));
			int result                 = ShaderManager::Programs[i]->LoadAndLink(vs.File, fs.File);

			if ((result != 0) || !ShaderManager::Programs[i]->IsOK())
				return -1;
		}

		break;
	}

	return 0;
}
