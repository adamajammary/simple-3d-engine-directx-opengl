#include "Terrain.h"

Terrain::Terrain(const std::vector<wxString> &textureImageFiles, int size, int octaves, float redistribution) : Component("Terrain")
{
	this->modelFile         = modelFile;
	this->textureImageFiles = textureImageFiles;
	this->type              = COMPONENT_TERRAIN;

	this->create(size, octaves, redistribution);
}

Terrain::Terrain()
{
	this->octaves        = 0.0f;
	this->redistribution = 0.0f;
	this->size           = 0.0f;
	this->type           = COMPONENT_TERRAIN;
}

void Terrain::create(int size, int octaves, float redistribution)
{
	RenderEngine::Canvas.Window->SetStatusText("Loading the Terrain ...");

	// https://www.dropbox.com/sh/do47b1opx7sxr2a/AAA9Wt05Lgqdm1z9g-YKyDWya/HeightsGenerator.java?dl=0
	// https://en.wikipedia.org/wiki/Perlin_noise
	// http://codeflow.org/entries/2011/nov/10/webgl-gpu-landscaping-and-erosion/
	// http://www.mbsoftworks.sk/index.php?page=tutorials&series=1
	// https://www.google.no/search?q=opengl+terrain+perlin+noise+vs+heightmap+image&source=lnms&sa=X&ved=0ahUKEwjYmYnMiM3UAhWGDZoKHXjHChMQ_AUICSgA&biw=1546&bih=937&dpr=1
	// https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
	// http://www.redblobgames.com/maps/terrain-from-noise/

	this->octaves        = octaves;
	this->redistribution = redistribution;
	this->size           = size;

	std::vector<unsigned int> indices;
	std::vector<float>        normals;
	std::vector<float>        textureCoords;
	std::vector<float>        vertices;
	int                       offset = (size / 2);
	float                     vertex1, vertex2, vertex3;

	for (int z = 0; z < size; z++) {
	for (int x = 0; x < size; x++)
	{
		vertex1 = (float)((float)x - (float)offset);
		vertex2 = Noise::Height(x, z, octaves, redistribution);
		vertex3 = (float)((float)z - (float)offset);

		vertices.push_back(vertex1);
		vertices.push_back(vertex2);
		vertices.push_back(vertex3);

		normals.push_back(vertex1);
		normals.push_back(vertex2);
		normals.push_back(vertex2);

		textureCoords.push_back((float)x / (float)size);
		textureCoords.push_back((float)z / (float)size);
	}
	}

	// http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=8
	//
	// VERTICES
	// 0, 1, 2, 3,
	// 4, 5, 6, 7 ...
	//
	// INDICES
	// 0, 4, 1,
	// 1, 4, 5
	// 
	// QUAD FACES (TRIANGLE 1 + TRIANGLE 2)
	for (int z = 0; z < size - 1; z++)
	{
		for (int x = 0; x < size - 1; x++)
		{
			int topLeft    = (((z + 0) * size) + x);   // CURRENT ROW
			int bottomLeft = (((z + 1) * size) + x);   // NEXT ROW

			// TRIANGLE 1
			indices.push_back(topLeft    + 0);	// TOP-LEFT
			indices.push_back(bottomLeft + 0);	// BOTTOM-LEFT
			indices.push_back(topLeft    + 1);	// TOP-RIGHT

			// TRIANGLE 2
			indices.push_back(topLeft    + 1);	// TOP-RIGHT
			indices.push_back(bottomLeft + 0);	// BOTTOM-LEFT
			indices.push_back(bottomLeft + 1);	// BOTTOM-RIGHT
		}
	}

	if (this->Children.empty())
		this->Children = { new Mesh(this, "Terrain") };

	this->isValid = (!this->Children.empty());

	if (this->isValid && (this->Children[0] != nullptr))
	{
		dynamic_cast<Mesh*>(this->Children[0])->LoadArrays(indices, normals, textureCoords, vertices);

		Texture* texture;

		for (int i = 0; i < 5; i++)
		{
			texture        = new Texture(this->textureImageFiles[i], true);
			texture->Scale = glm::vec2(size, size);

			this->Children[0]->LoadTexture(texture, i);
		}

		RenderEngine::Canvas.Window->SetStatusText("Loading the Terrain ... OK");
	} else {
		wxMessageBox("ERROR: Failed to load the Terrain.", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		RenderEngine::Canvas.Window->SetStatusText("Loading the Terrain ... FAIL");
	}
}

int Terrain::Octaves()
{
    return this->octaves;
}

void Terrain::Resize(int size, int octaves, float redistribution)
{
    this->create(size, octaves, redistribution);
}

float Terrain::Redistribution()
{
    return this->redistribution;
}

int Terrain::Size()
{
    return this->size;
}
