#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_MESH_H
#define S3DE_MESH_H

class Mesh : public Component
{
public:
	Mesh(Component* parent, const wxString &name);
	Mesh();
	virtual ~Mesh();

protected:
	std::vector<unsigned int> indices;
	std::vector<float>        normals;
	std::vector<float>        textureCoords;
	std::vector<float>        vertices;
	Buffer*                   indexBuffer;
	Buffer*                   normalBuffer;
	Buffer*                   textureCoordsBuffer;
	Buffer*                   vertexBuffer;

private:
	BoundingVolume* boundingVolume;
	bool            isSelected;
	float           maxScale;

public:
	void            BindBuffer(GLuint bufferID, GLuint shaderAttrib, GLsizei size, GLenum arrayType, GLboolean normalized, const GLvoid* offset = nullptr);
	BoundingVolume* GetBoundingVolume();
	Buffer*         IndexBuffer();
	Buffer*         VertexBuffer();
	GLuint          IBO();
	GLuint          NBO();
	GLuint          TBO();
	GLuint          VBO();
	bool            IsOK();
	bool            IsSelected();
	bool            LoadArrays(std::vector<unsigned int> &indices, std::vector<float> &normals, std::vector<float> &textureCoords, std::vector<float> &vertices);
	bool            LoadModelFile(aiMesh* mesh, const aiMatrix4x4 &transformMatrix);
	//void            LoadTexture(Texture* texture, int index);
	int             LoadTextureImage(const wxString &imageFile, int index);
	void            MoveBy(const glm::vec3 &amount)      override;
	void            MoveTo(const glm::vec3 &newPosition) override;
	size_t          NrOfIndices();
	size_t          NrOfVertices();
	void            RemoveTexture(int index);
	void            Select(bool selected);
	void            SetBoundingVolume(BoundingVolumeType type);
	void            UpdateBoundingVolume();

protected:
	bool loadModelData(aiMesh* mesh);
	bool setModelData();
	void updateModelData();

private:
	void setMaxScale();
	void updateModelData(const aiVector3D &position, const aiVector3D &scale, aiVector3D &rotation);

};

#endif
