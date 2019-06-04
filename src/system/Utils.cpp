#include "Utils.h"

const wxChar* Utils::ALIGNMENTS[] = {
    wxT("Top-Left"),
	wxT("Top-Center"),
	wxT("Top-Right"),
	wxT("Middle-Left"),
	wxT("Middle-Center"),
	wxT("Middle-Right"),
	wxT("Bottom-Left"),
	wxT("Bottom-Center"),
	wxT("Bottom-Right"),
	nullptr
};

const wxString Utils::APP_NAME           = "Simple 3D Engine";
const uint8_t  Utils::APP_VERSION_MAJOR  = 1;
const uint8_t  Utils::APP_VERSION_MINOR  = 0;
const uint8_t  Utils::APP_VERSION_PATCH  = 2;
const wxString Utils::APP_VERSION        = "1.0.2";
const wxString Utils::ASPECT_RATIOS[]    = { "16:9", "4:3" };
const wxChar*  Utils::BOUNDING_VOLUMES[] = { wxT("none"), wxT("box"), wxT("sphere"), nullptr };
const wxString Utils::COPYRIGHT          = "\u00A9 2017 Adam A. Jammary";
const wxString Utils::TESTED             = "Tested on Windows 10 (64-bit)";
const wxString Utils::GOOGLE_ADS_URL     = "http://jammary.com/google_ad_horizontal.html";
const wxString Utils::DRAW_MODES[]       = { "Filled", "Wireframe" };
const wxString Utils::FOVS[]             = { "45\u00B0", "60\u00B0", "75\u00B0", "90\u00B0" };
const wxString Utils::IMAGE_FILE_FORMATS = "All supported formats|*.bmp;*.png;*.jpg;*.tif;*.gif;*.pnm;*.pcx;*.ico;*.cur;*.ani;*.tga;*.xpm";
const wxString Utils::SCENE_FILE_FORMAT  = "Scene file (*.scene)|*.scene";
const wxSize   Utils::UI_ADS_SIZE        = wxSize(730,  90);
const wxSize   Utils::UI_LIST_BOX_SIZE   = wxSize(290,  150);
const wxSize   Utils::UI_RENDER_SIZE     = wxSize(640,  360);
const wxSize   Utils::UI_WINDOW_SIZE     = wxSize(1280, 875);
const wxSize   Utils::UI_PROPS_SIZE      = wxSize(590,  280);
const wxSize   Utils::UI_TABS_SIZE       = wxSize(1245, 85);

#if defined _WINDOWS
	const size_t Utils::NR_OF_GRAPHICS_APIS = 4;
#else
	const size_t Utils::NR_OF_GRAPHICS_APIS = 2;
#endif

const wxString Utils::GRAPHIC_APIS[NR_OF_GRAPHICS_APIS] = {
	"OpenGL", "Vulkan"
	#if defined _WINDOWS
	, "DirectX 11", "DirectX 12"
	#endif
};

const wxChar* Utils::FONTS[] = {
	wxT("Arial"),
	wxT("Arial Black"),
	wxT("Comic Sans MS"),
	wxT("Courier New"),
	wxT("Georgia"),
	wxT("Impact"),
	wxT("Lucida Console"),
	wxT("Lucida Sans Unicode"),
	wxT("Palatino Linotype"),
	wxT("Tahoma"),
	wxT("Times New Roman"),
	wxT("Trebuchet MS"),
	wxT("Verdana"),
	nullptr
};

const std::vector<Icon> Utils::ICONS_GEOMETRY = {
	{ "img/icon-plane-128.png",         ID_ICON_PLANE,       "Plane" },
	{ "img/icon-cube-100.png",          ID_ICON_CUBE,        "Cube" },
	{ "img/icon-uvsphere-100.png",      ID_ICON_UV_SPHERE,   "UV Sphere" },
	{ "img/icon-icosphere-100-2.png",   ID_ICON_ICO_SPHERE,  "Ico Sphere" },
	{ "img/icon-cylinder-100.png",      ID_ICON_CYLINDER,    "Cylinder" },
	{ "img/icon-cone-128-1.png",        ID_ICON_CONE,        "Cone" },
	{ "img/icon-torus-256.png",         ID_ICON_TORUS,       "Torus" },
	{ "img/icon-monkeyhead-100x83.png", ID_ICON_MONKEY_HEAD, "Monkey" }
};

const std::vector<Icon> Utils::ICONS_ENVIRONMENT = {
	{ "img/icon-skybox-256.png",      ID_ICON_SKYBOX,  "Skybox" },
	{ "img/icon-terrain-200x121.png", ID_ICON_TERRAIN, "Terrain" },
	{ "img/icon-water-208x235.png",   ID_ICON_WATER,   "Water" }
};

const std::vector<Icon> Utils::ICONS_LIGHTS = {
	{ "img/icon-light-directional-128.png", ID_ICON_LIGHT_DIRECTIONAL, "Directional Light" },
	{ "img/icon-light-point-128.png",       ID_ICON_LIGHT_POINT,       "Point Light" },
	{ "img/icon-light-spot-2-128.png",      ID_ICON_LIGHT_SPOT,        "Spot Light" }
};

const std::vector<Icon> Utils::ICONS_UI = {
	{ "img/icon-hud-128.png", ID_ICON_HUD, "HUD" }
};

const wxString Utils::MODEL_FILE_FORMATS =
	"All supported formats|*.mdl;*.hmp;*.3ds;*.ase;*.ac;*.dxf;*.fbx;*.bvh;*.blend;*.csm;*.dae;*.xml;*.x;*.md5mesh;*.md5anim;*.md5camera;*.ifc;*.irrmesh;*.xml;*.irr;*.xml;*.lwo;*.lws;*.ms3d;*.lxo;*.nff;*.off;*.mesh.xml;*.skeleton.xml;*.material;*.mdl;*.md2;*.md3;*.q3o;*.q3s;*.raw;*.mdc;*.nff;*.ply;*.stl;*.ter;*.smd;*.vta;*.obj;*.xgl;*.zgl|"
	"3D GameStudio Model (*.mdl)|*.mdl|"
	"3D GameStudio Terrain (*.hmp)|*.hmp|"
	"3D Studio Max 3DS (*.3ds)|*.3ds|"
	"3D Studio Max ASE (*.ase)|*.ase|"
	"AC3D (*.ac)|*.ac|"
	"AutoCAD DXF (*.dxf)|*.dxf|"
	"Autodesk (*.fbx)|*.fbx|"
	"Biovision BVH (*.bvh)|*.bvh|"
	"Blender (*.blend)|*.blend|"
	"CharacterStudio Motion (*.csm)|*.csm|"
	"Collada (*.dae;*.xml)|*.dae;*.xml|"
	"DirectX X (*.x)|*.x|"
	"Doom 3 (*.md5mesh;*.md5anim;*.md5camera)|*.md5mesh;*.md5anim;*.md5camera|"
	"IFC - STEP, Industry Foundation Classes (*.ifc)|*.ifc|"
	"Irrlicht Mesh (*.irrmesh;*.xml)|*.irrmesh;*.xml|"
	"Irrlicht Scene (*.irr;*.xml)|*.irr;*.xml|"
	"LightWave Model (*.lwo)|*.lwo|"
	"LightWave Scene (*.lws)|*.lws|"
	"Milkshape 3D (*.ms3d)|*.ms3d|"
	"Modo Model (*.lxo)|*.lxo|"
	"Neutral File Format (*.nff)|*.nff|"
	"Object File Format (*.off)|*.off|"
	"Ogre (*.mesh.xml;*.skeleton.xml;*.material)|*.mesh.xml;*.skeleton.xml;*.material|"
	"Quake I (*.mdl)|*.mdl|"
	"Quake II (*.md2)|*.md2|"
	"Quake III (*.md3)|*.md3|"
	"Quick3D (*.q3o;*.q3s)|*.q3o;*.q3s|"
	"Raw Triangles (*.raw)|*.raw|"
	"RtCW (*.mdc)|*.mdc|"
	"Sense8 WorldToolkit (*.nff)|*.nff|"
	"Stanford Polygon Library (*.ply)|*.ply|"
	"Stereolithography (*.stl)|*.stl|"
	"Terragen Terrain (*.ter)|*.ter|"
	"Valve Model (*.smd;*.vta)|*.smd;*.vta|"
	"Wavefront Object (*.obj)|*.obj|"
	"XGL (*.xgl;*.zgl)|*.xgl;*.zgl"
;

wxString Utils::PROPERTY_IDS[NR_OF_PROPERTY_IDS] = {
	"PROPERTY_ID_NAME",
	"PROPERTY_ID_LOCATION",
	"PROPERTY_ID_LOCATION_",
	"PROPERTY_ID_ROTATION",
	"PROPERTY_ID_ROTATION_",
	"PROPERTY_ID_SCALE",
	"PROPERTY_ID_SCALE_",
	"PROPERTY_ID_AUTO_ROTATION",
	"PROPERTY_ID_AUTO_ROTATION_",
	"PROPERTY_ID_ENABLE_AUTO_ROTATION",
	"PROPERTY_ID_COLOR",
	"PROPERTY_ID_SPEC_INTENSITY",
	"PROPERTY_ID_SPEC_SHININESS",
	"PROPERTY_ID_OPACITY",
	"PROPERTY_ID_TRANSPARENCY",
	"PROPERTY_ID_TEXT",
	"PROPERTY_ID_TEXT_ALIGNMENT",
	"PROPERTY_ID_TEXT_FONT",
	"PROPERTY_ID_TEXT_SIZE",
	"PROPERTY_ID_TEXT_COLOR",
	"PROPERTY_ID_HUD_TEXTURE",
	"PROPERTY_ID_HUD_REMOVE_TEXTURE",
	"PROPERTY_ID_TEXTURE_",
	"PROPERTY_ID_REMOVE_TEXTURE_",
	"PROPERTY_ID_FLIP_TEXTURE_",
	"PROPERTY_ID_REPEAT_TEXTURE_",
	"PROPERTY_ID_TILING_U_",
	"PROPERTY_ID_TILING_V_",
	"PROPERTY_ID_BOUNDING_VOLUME",
	"PROPERTY_ID_TERRAIN_SIZE",
	"PROPERTY_ID_TERRAIN_OCTAVES",
	"PROPERTY_ID_TERRAIN_REDISTRIBUTION",
	"PROPERTY_ID_WATER_SPEED",
	"PROPERTY_ID_WATER_WAVE_STRENGTH",
	"PROPERTY_ID_LIGHT_ACTIVE",
	"PROPERTY_ID_LIGHT_LOCATION",
	"PROPERTY_ID_LIGHT_LOCATION_",
	"PROPERTY_ID_LIGHT_AMBIENT",
	"PROPERTY_ID_LIGHT_DIFFUSE",
	"PROPERTY_ID_LIGHT_SPEC_INTENSITY",
	"PROPERTY_ID_LIGHT_SPEC_SHININESS",
	"PROPERTY_ID_LIGHT_DIRECTION",
	"PROPERTY_ID_LIGHT_DIRECTION_",
	"PROPERTY_ID_LIGHT_ATT_LINEAR",
	"PROPERTY_ID_LIGHT_ATT_QUAD",
	"PROPERTY_ID_LIGHT_ANGLE_INNER",
	"PROPERTY_ID_LIGHT_ANGLE_OUTER"
};

std::map<wxString, wxString> Utils::RESOURCE_IMAGES = {
	{ "emptyTexture",      "resources/textures/white_1x1.png" },
	{ "backgroundTexture", "resources/textures/terrain/backgroundTexture.png" },
	{ "rTexture",          "resources/textures/terrain/rTexture.png" },
	{ "gTexture",          "resources/textures/terrain/gTexture.png" },
	{ "bTexture",          "resources/textures/terrain/bTexture.png" },
	{ "blendMap",          "resources/textures/terrain/blendMap.png" },
	{ "duDvMap",           "resources/textures/water/duDvMap.png" },
	{ "normalMap",         "resources/textures/water/normalMap.png" },
	{ "skyboxRight",       "resources/textures/skybox/right.png" },
	{ "skyboxLeft",        "resources/textures/skybox/left.png" },
	{ "skyboxTop",         "resources/textures/skybox/top.png" },
	{ "skyboxBottom",      "resources/textures/skybox/bottom.png" },
	{ "skyboxBack",        "resources/textures/skybox/back.png" },
	{ "skyboxFront",       "resources/textures/skybox/front.png" }
};

std::map<IconType, wxString> Utils::RESOURCE_MODELS = {
	{ ID_ICON_QUAD,        "resources/models/quad.blend" },
	{ ID_ICON_PLANE,       "resources/models/plane.blend" },
	{ ID_ICON_CUBE,        "resources/models/cube.blend" },
	{ ID_ICON_UV_SPHERE,   "resources/models/uv_sphere.blend" },
	{ ID_ICON_ICO_SPHERE,  "resources/models/ico_sphere.blend" },
	{ ID_ICON_CYLINDER,    "resources/models/cylinder.blend" },
	{ ID_ICON_CONE,        "resources/models/cone.blend" },
	{ ID_ICON_TORUS,       "resources/models/torus.blend" },
	{ ID_ICON_MONKEY_HEAD, "resources/models/monkey_head.blend" }
};

const std::vector<Resource> Utils::SHADER_RESOURCES_DX = {
	{ "resources/shaders/color.hlsl",   "color",     "" },
	{ "resources/shaders/default.hlsl", "default",   "" },
	{ "resources/shaders/depth.hlsl",   "depth",     "" },
	{ "resources/shaders/hud.hlsl",     "hud",       "" },
	{ "resources/shaders/skybox.hlsl",  "skybox",    "" },
	{ "resources/shaders/color.hlsl",   "wireframe", "" }
};

const std::vector<Resource> Utils::SHADER_RESOURCES_GL_VK = {
	{ "resources/shaders/color.vs.glsl",   "color_vs",     "" },
	{ "resources/shaders/color.fs.glsl",   "color_fs",     "" },
	{ "resources/shaders/default.vs.glsl", "default_vs",   "" },
	{ "resources/shaders/default.fs.glsl", "default_fs",   "" },
	{ "resources/shaders/depth.vs.glsl",   "depth_vs",     "" },
	{ "resources/shaders/depth.fs.glsl",   "depth_fs",     "" },
	{ "resources/shaders/hud.vs.glsl",     "hud_vs",       "" },
	{ "resources/shaders/hud.fs.glsl",     "hud_fs",       "" },
	{ "resources/shaders/skybox.vs.glsl",  "skybox_vs",    "" },
	{ "resources/shaders/skybox.fs.glsl",  "skybox_fs",    "" },
	{ "resources/shaders/color.vs.glsl",   "wireframe_vs", "" },
	{ "resources/shaders/color.fs.glsl",   "wireframe_fs", "" }
};

std::vector<uint8_t> Utils::Compress(const std::vector<uint8_t> &data)
{
	std::vector<uint8_t> outBuffer;

	if (data.empty())
		return outBuffer;

	size_t headerSize = LZMA_PROPS_SIZE;
	size_t outSize    = (data.size() + (data.size() * 0.1));

	outBuffer.resize(headerSize + outSize);

	int result = LzmaCompress(
		&outBuffer[LZMA_PROPS_SIZE], &outSize, &data[0], data.size(), &outBuffer[0], &headerSize,
		-1, 0, -1, -1, -1, -1, -1
	);

	if ((headerSize == LZMA_PROPS_SIZE) && (result == SZ_OK))
		outBuffer.resize(headerSize + outSize);
	else
		outBuffer.clear();

	return outBuffer;
}

std::vector<uint8_t> Utils::Decompress(const std::vector<uint8_t> &data)
{
	std::vector<uint8_t> outBuffer;

	if ((data.size() < LZMA_OFFSET_ID + LZMA_OFFSET_SIZE) || (std::string(data.begin(), data.begin() + 5) != "@LZMA"))
		return outBuffer;

	// HEADER: first 8 bytes - file type identifier
	// HEADER: next  8 bytes - size of the uncompressed data
	const int HEADER_OFFSET = (LZMA_OFFSET_ID + LZMA_OFFSET_SIZE);
	int64_t   outSize       = 0;
	size_t    inSize        = (data.size() - LZMA_PROPS_SIZE);

	for (int i = (HEADER_OFFSET - 1); i >= LZMA_OFFSET_ID; i--)
		outSize += (data[i] * std::pow((0xFF + 1), ((HEADER_OFFSET - 1) - i)));

	outBuffer.resize(outSize);

	int result = LzmaUncompress(
		&outBuffer[0], reinterpret_cast<size_t*>(&outSize), &data[HEADER_OFFSET + LZMA_PROPS_SIZE],
		&inSize, &data[HEADER_OFFSET], LZMA_PROPS_SIZE
	);

	if ((result == SZ_OK) || (result == SZ_ERROR_INPUT_EOF))
		outBuffer.resize(outSize);
	else
		outBuffer.clear();

	return outBuffer;
}

#if defined _WINDOWS
DXGI_FORMAT Utils::GetImageFormatDXGI(const wxImage &image, bool srgb)
{
	return (srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
}
#endif

wxString Utils::GetGraphicsAPI(GraphicsAPI api)
{
	wxString apiString = "";

	switch (RenderEngine::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11: apiString = "DirectX 11"; break;
		case GRAPHICS_API_DIRECTX12: apiString = "DirectX 12"; break;
		#endif
		case GRAPHICS_API_OPENGL:    apiString = "OpenGL"; break;
		case GRAPHICS_API_VULKAN:    apiString = "Vulkan"; break;
		default: throw;
	}

	return apiString;
}

GLenum Utils::GetImageFormat(const wxImage &image, bool srgb, bool in)
{
	return (in ? (srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8) : GL_RGBA);
}

VkFormat Utils::GetImageFormatVK(const wxImage &image, bool srgb)
{
	return (srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM);
}

GLsizei Utils::GetStride(GLsizei size, GLenum arrayType)
{
	GLsizei stride = 0;

	switch (arrayType) {
		case GL_BYTE:           stride = (size * sizeof(char));           break;
		case GL_UNSIGNED_BYTE:  stride = (size * sizeof(unsigned char));  break;
		case GL_SHORT:          stride = (size * sizeof(short));          break;
		case GL_UNSIGNED_SHORT: stride = (size * sizeof(unsigned short)); break;
		case GL_INT:            stride = (size * sizeof(int));            break;
		case GL_UNSIGNED_INT:   stride = (size * sizeof(unsigned int));   break;
		case GL_FLOAT:          stride = (size * sizeof(float));          break;
		default: throw;
	}

	return stride;
}

std::vector<uint8_t> Utils::LoadDataFile(const wxString &file)
{
	std::vector<uint8_t> result;
	size_t               size;
	std::ifstream        fileStream(file.wc_str(), (std::ios::binary | std::ios::ate));

	if (!fileStream.good())
	{
		wxMessageBox("ERROR: Failed to load " + file, RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		fileStream.close();

		return result;
	}

	//fileStream.seekg(0, std::ios::end);
	size = fileStream.tellg();
	//fileStream.seekg(0, std::ios::beg);
	fileStream.seekg(0);

	result.resize(size);
	fileStream.read(reinterpret_cast<char*>(&result[0]), size);
	fileStream.close();

	return result;
}

wxImage* Utils::LoadImageFile(const wxString &file, wxBitmapType type)
{
	wxImage* image = new wxImage(file, type);

	if ((image != nullptr) && image->IsOk())
		return image;

	return nullptr;
}

std::vector<AssImpMesh*> Utils::LoadModelFile(const wxString &file)
{
	std::vector<AssImpMesh*> meshes;
	const aiScene*           scene = aiImportFile(file.c_str(), (aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_ImproveCacheLocality | aiProcess_OptimizeMeshes));

	if ((scene == nullptr) || !scene->HasMeshes() || (scene->mNumMeshes == 0))
	{
		wxMessageBox("ERROR! Failed to load " + file + "\n\n" + aiGetErrorString(), RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);

		if (scene != nullptr)
			aiReleaseImport(scene);

		return meshes;
	}

	AssImpMesh*  mesh;
	aiNode*      node;
	uint32_t     nrOfChildren = (scene->mRootNode->mNumChildren > 0 ? scene->mRootNode->mNumChildren : 1);

	// SCENE CHILDREN
	for (uint32_t i = 0; i < nrOfChildren; i++)
	{
		node = (scene->mRootNode->mNumChildren > 0 ? scene->mRootNode->mChildren[i] : scene->mRootNode);

		// CHILD MESHES
		for (uint32_t j = 0; j < node->mNumMeshes; j++)
		{
			mesh                 = new AssImpMesh();
			mesh->Mesh           = scene->mMeshes[node->mMeshes[j]];
			mesh->Scene          = scene;
			mesh->Transformation = node->mTransformation;

			mesh->Name = mesh->Mesh->mName.C_Str();
			mesh->Name = (!mesh->Name.empty() ? mesh->Name : scene->mRootNode->mChildren[i]->mName.C_Str());
			mesh->Name = (!mesh->Name.empty() ? mesh->Name : "Mesh");

			// MESH MATERIALS: http://assimp.sourceforge.net/lib_html/materials.html
			if (scene->mNumMaterials > 0)
			{
				aiString    textures[MAX_TEXTURES] = {};
				aiMaterial* material      = scene->mMaterials[mesh->Mesh->mMaterialIndex];
				aiColor4D   diffuse       = {};
				aiColor3D   specIntensity = {};
				float       specShininess = 0;
				size_t      pathSeparator = file.rfind("/");

				if (pathSeparator == wxString::npos)
					pathSeparator = file.rfind("\\");

				wxString path = file.substr(0, pathSeparator + 1);

				material->Get(AI_MATKEY_COLOR_DIFFUSE,  diffuse);
				material->Get(AI_MATKEY_COLOR_SPECULAR, specIntensity);
				material->Get(AI_MATKEY_SHININESS,      specShininess);
				material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE,  0), textures[0]);
				material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), textures[1]);

				mesh->MeshMaterial.diffuse            = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };
				mesh->MeshMaterial.specular.intensity = { specIntensity.r, specIntensity.g, specIntensity.b };
				mesh->MeshMaterial.specular.shininess = specShininess;

				if (textures[0].length > 0)
					mesh->MeshMaterial.textures[0] = (path + textures[0].C_Str());

				if (textures[1].length > 0)
					mesh->MeshMaterial.textures[1] = (path + textures[1].C_Str());
			}

			if (mesh->Mesh != nullptr)
				meshes.push_back(mesh);
		}
	}

	return meshes;
}

std::vector<Component*> Utils::LoadModelFile(const wxString &file, Component* parent)
{
	std::vector<Component*>  children;
	Mesh*                    mesh;
	std::vector<AssImpMesh*> aiMeshes = Utils::LoadModelFile(file);

	for (auto aiMesh : aiMeshes)
	{
		mesh = new Mesh(parent, aiMesh->Name);

		if (mesh == nullptr)
			continue;

		mesh->ComponentMaterial = aiMesh->MeshMaterial;

		mesh->LoadModelFile(aiMesh->Mesh, aiMesh->Transformation);

		if (!mesh->IsValid()) {
			_DELETEP(mesh);
			continue;
		}

		children.push_back(mesh);
	}

	if (!aiMeshes.empty())
		aiReleaseImport(aiMeshes[0]->Scene);

	return children;
}

wxString Utils::LoadTextFile(const wxString &file)
{
	if (file.empty())
		return "";

	std::wstring   line;
	wxString       result = "";
	std::wifstream fileStream(file.wc_str());

	if (!fileStream.good())
	{
		wxMessageBox("ERROR: Failed to load " + file, RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		fileStream.close();

		return "";
	}

	while (std::getline(fileStream, line))
		result.append(line + "\n");

	fileStream.close();

	return result;
}

wxString Utils::OpenFileDialog(const wxString &fileFormats, bool save)
{
	long         flags        = (save ? (wxFD_SAVE | wxFD_OVERWRITE_PROMPT) : (wxFD_OPEN | wxFD_FILE_MUST_EXIST));
	wxString     selectedFile = "";
	wxFileDialog fileDialog(RenderEngine::Canvas.Window, "", "", "", fileFormats, flags);

	if (fileDialog.ShowModal() != wxID_CANCEL)
		selectedFile = fileDialog.GetPath();;

	return selectedFile;
}

wxString Utils::OpenFile(const wxString &fileFormats)
{
	return Utils::OpenFileDialog(fileFormats, false);
}

wxString Utils::SaveFile(const wxString &fileFormats)
{
	return Utils::OpenFileDialog(fileFormats, true);
}

int Utils::SaveDataToFile(const std::vector<uint8_t> &data, const wxString &file, uint64_t size)
{
	if (file.empty())
		return -1;

	std::ofstream fileStream(file.wc_str(), std::ios::binary);

	if (!fileStream.good())
	{
		wxMessageBox("ERROR: Failed to save to " + file, RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		fileStream.close();

		return -1;
	}


	// HEADER: first 8 bytes - file type identifier
	char headerFileID[LZMA_OFFSET_ID] = { '@', 'L', 'Z', 'M', 'A', 0, 0, 0 };
	fileStream.write(headerFileID, LZMA_OFFSET_ID);

	// HEADER: next 8 bytes - size of the uncompressed data
	for (int i = LZMA_OFFSET_SIZE - 1; i >= 0; i--)
	{
		if (size > 0)
			fileStream.put(Utils::ToByte(size, i));
		else
			fileStream.put(0);
	}

	fileStream.write(reinterpret_cast<const char*>(&data[0]), data.size());
	fileStream.close();

	return 0;
}

int Utils::SaveTextToFile(const wxString &text, const wxString &file)
{
	if (file.empty())
		return -1;

	std::wofstream fileStream(file.wc_str());

	if (!fileStream.good())
	{
		wxMessageBox("ERROR: Failed to save to " + file, RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		fileStream.close();

		return -1;
	}

	fileStream << file.wc_str();
	fileStream.close();

	return 0;
}

uint8_t Utils::ToByte(uint64_t number, int byteIndex)
{
	return ((number >> (byteIndex * 8)) & 0xFF);
}

float Utils::ToRadians(float degrees)
{
    return (degrees * glm::pi<float>() / 180.0f);
}

float Utils::ToDegrees(float radians)
{
	return (radians * 180.0f / glm::pi<float>());
}

float Utils::ToFloat(bool boolean)
{
	return (boolean ? 1.0f : 0);
}

wxFloatProperty* Utils::ToFloatProperty(float value, const wxString &label, const wxString &id, float min, float max)
{
	wxFloatProperty* floatProperty = new wxFloatProperty(label, id, value);

	if (min >= 0.0f)
		floatProperty->SetAttribute(wxPG_ATTR_MIN, min);

	if (max >= 0.0f)
		floatProperty->SetAttribute(wxPG_ATTR_MAX, max);

	return floatProperty;
}

json11::Json::array Utils::ToJsonArray(const glm::vec2 &arr)
{
	json11::Json::array jsonArray = { arr[0], arr[1] };
	return jsonArray;
}

json11::Json::array Utils::ToJsonArray(const glm::vec3 &arr)
{
	json11::Json::array jsonArray = { arr[0], arr[1], arr[2] };
	return jsonArray;
}

json11::Json::array Utils::ToJsonArray(const glm::vec4 &arr)
{
	json11::Json::array jsonArray = { arr[0], arr[1], arr[2], arr[3] };
	return jsonArray;
}

uint8_t* Utils::ToRGBA(const wxImage &image)
{
	int      size  = (image.GetWidth() * image.GetHeight() * 4);
	uint8_t* rgb   = image.GetData();
	uint8_t* alpha = image.GetAlpha();
	uint8_t* rgba  = (uint8_t*)std::malloc(size);

	if ((rgb == nullptr) || (rgba == nullptr))
		return nullptr;

	for (int i = 0, r = 0, a = 0; i < size; i += 4, r += 3, a++)
	{
		rgba[i + 0] = rgb[r + 0];
		rgba[i + 1] = rgb[r + 1];
		rgba[i + 2] = rgb[r + 2];
		rgba[i + 3] = (alpha != nullptr ? alpha[a] : 0xFF);
	}

	return rgba;
}

std::string Utils::ToString(const std::vector<uint8_t> &data)
{
	if (data.empty())
		return "";

	const char* data2  = reinterpret_cast<const char*>(data.data());
	std::string result = std::string(data2);

	if (result.size() > data.size())
		result.resize(data.size());

	return result;
}

glm::vec2 Utils::ToVec2(const json11::Json::array &jsonArray)
{
	glm::vec2 result = { jsonArray[0].number_value(), jsonArray[1].number_value() };
	return result;
}

glm::vec3 Utils::ToVec3(const json11::Json::array &jsonArray)
{
	glm::vec3 result = { jsonArray[0].number_value(), jsonArray[1].number_value(), jsonArray[2].number_value() };
	return result;
}

glm::vec4 Utils::ToVec4(const json11::Json::array &jsonArray)
{
	glm::vec4 result = { jsonArray[0].number_value(), jsonArray[1].number_value(), jsonArray[2].number_value(), jsonArray[3].number_value() };
	return result;
}

glm::vec3 Utils::ToVec3Color(const wxColour &color)
{
	glm::vec3 vec3Colour = glm::vec3(
		(color.Red() / 255.0f), (color.Green() / 255.0f), (color.Blue() / 255.0f)
	);
	return vec3Colour;
}

glm::vec4 Utils::ToVec4Color(const wxColour &color)
{
	glm::vec4 vec4Colour = glm::vec4(
		(color.Red() / 255.0f), (color.Green() / 255.0f), (color.Blue() / 255.0f), (color.Alpha() / 255.0f)
	);
	return vec4Colour;
}

glm::vec4 Utils::ToVec4Float(bool boolean)
{
	float value = (boolean ? 1.0f : 0);
	return glm::vec4(value, value, value, value);
}

glm::vec4 Utils::ToVec4Float(int integer)
{
	float value = static_cast<float>(integer);
	return glm::vec4(value, value, value, value);
}

std::vector<float> Utils::ToVertexBufferData(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords)
{
	std::vector<float> data;

	for (int i = 0, j = 0; i < (int)vertices.size(); i += 3, j += 2)
	{
		if ((int)normals.size() > i + 2) {
			data.push_back(normals[i + 0]);
			data.push_back(normals[i + 1]);
			data.push_back(normals[i + 2]);
		}

		if ((int)vertices.size() > i + 2) {
			data.push_back(vertices[i + 0]);
			data.push_back(vertices[i + 1]);
			data.push_back(vertices[i + 2]);
		}

		if ((int)texCoords.size() > j + 1) {
			data.push_back(texCoords[j + 0]);
			data.push_back(texCoords[j + 1]);
		}
	}

	return data;
}

wxColour Utils::ToWxColour(const wxVariant &color)
{
	wxColour colorWX;
	colorWX << color;
	return colorWX;
}

wxColour Utils::ToWxColour(const glm::vec3 &color)
{
	wxColour colorWX(
		(uint8_t)(color.r * 255.0f), (uint8_t)(color.g * 255.0f), (uint8_t)(color.b * 255.0f), 0xFF
	);
	return colorWX;
}

wxColour Utils::ToWxColour(const glm::vec4 &color)
{
	wxColour colorWX(
		(uint8_t)(color.r * 255.0f), (uint8_t)(color.g * 255.0f), (uint8_t)(color.b * 255.0f), (uint8_t)(color.a * 255.0f)
	);
	return colorWX;
}

DirectX::XMFLOAT2 Utils::ToXMFLOAT2(const glm::vec2 &vector)
{
	return DirectX::XMFLOAT2(reinterpret_cast<const float*>(&vector[0]));
}

DirectX::XMFLOAT3 Utils::ToXMFLOAT3(const glm::vec3 &vector)
{
	return DirectX::XMFLOAT3(reinterpret_cast<const float*>(&vector[0]));
}

DirectX::XMFLOAT4 Utils::ToXMFLOAT4(const glm::vec3 &vector, float w)
{
	return DirectX::XMFLOAT4(vector.x, vector.y, vector.z, w);
}

DirectX::XMFLOAT4 Utils::ToXMFLOAT4(const glm::vec4 &vector)
{
	return DirectX::XMFLOAT4(reinterpret_cast<const float*>(&vector[0]));
}

DirectX::XMFLOAT4 Utils::ToXMFLOAT4(bool boolean)
{
	float value = (boolean ? 1.0f : 0);
	return DirectX::XMFLOAT4(value, value, value, value);
}

DirectX::XMFLOAT4 Utils::ToXMFLOAT4(int integer)
{
	float value = static_cast<int>(integer);
	return DirectX::XMFLOAT4(value, value, value, value);
}

DirectX::XMMATRIX Utils::ToXMMATRIX(const glm::mat4 &matrix)
{
	return XMMatrixTranspose(DirectX::XMMATRIX(reinterpret_cast<const float*>(&matrix[0])));
}
