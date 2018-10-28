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

	const int nr_of_shaders = 4;

	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		for (int i = 0; i < NR_OF_SHADERS; i++)
		{
			ShaderManager::Programs[i] = new ShaderProgram(Utils::SHADER_RESOURCES_DX[i].Name);
			int result                 = ShaderManager::Programs[i]->Load(Utils::SHADER_RESOURCES_DX[i].File);

			if ((result < 0) || !ShaderManager::Programs[i]->IsOK())
				return -1;
		}

		break;
	#endif
	// TODO: Shaders GL+VK
	case GRAPHICS_API_OPENGL:
	case GRAPHICS_API_VULKAN:
		if ((int)Utils::SHADER_RESOURCES_GL_VK.size() < nr_of_shaders * 2)
			return -1;

		for (int i = 0; i < nr_of_shaders; i++)
		//if ((int)Utils::SHADER_RESOURCES_GL.size() < NR_OF_SHADERS * 2)
		//	return -1;

		//for (int i = 0; i < NR_OF_SHADERS; i++)
		{
			Resource vs = Utils::SHADER_RESOURCES_GL_VK[(i * 2) + 0];
			Resource fs = Utils::SHADER_RESOURCES_GL_VK[(i * 2) + 1];

			if (vs.Name.rfind("_vs") == wxString::npos)
				continue;
		
			vs.Result = Utils::LoadTextFile(vs.File);
			fs.Result = Utils::LoadTextFile(fs.File);

			ShaderManager::Programs[i] = new ShaderProgram(vs.Name.substr(0, vs.Name.rfind("_")));

			int result;
			
			if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
				result = ShaderManager::Programs[i]->LoadAndLink(vs.Result, fs.Result);
			else
				result = ShaderManager::Programs[i]->LoadAndLink(vs.File, fs.File);

			if ((result < 0) || !ShaderManager::Programs[i]->IsOK()) {
				ShaderManager::Programs[i]->Log();
				return result;
			}
		}

		break;
	}

	return 0;
}
