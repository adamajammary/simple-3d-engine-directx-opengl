#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_COMPONENT_H
#define S3DE_COMPONENT_H

class Component
{
public:
	Component(const wxString &name = "", const glm::vec3 &position = {});
	virtual ~Component();

public:
	bool                    AutoRotate;
	glm::vec3               AutoRotation;
	std::vector<Component*> Children;
	Material                ComponentMaterial;
	//bool                    LockToParentPosition;
	//bool                    LockToParentRotation;
	//bool                    LockToParentScale;
	wxString                Name;
	Component*              Parent;
	Texture*                Textures[MAX_TEXTURES];

protected:
	bool          isValid;
	glm::mat4     matrix;
	wxString      modelFile;
	glm::vec3     rotation;
	glm::mat4     rotationMatrix;
	glm::vec3     scale;
	glm::mat4     scaleMatrix;
	glm::vec3     position;
	glm::mat4     translationMatrix;
	ComponentType type;

public:
	int           GetChildIndex(Component* child);
	bool          IsTextured(int index);
	bool          IsValid();
	void          LoadTexture(Texture* texture, int index);
	glm::mat4     Matrix();
	wxString      ModelFile();
	virtual void  MoveBy(const glm::vec3 &amount);
	virtual void  MoveTo(const glm::vec3 &newPosition);
	glm::vec3     Position();
	int           RemoveChild(Mesh* child);
	glm::vec3     Rotation();
	virtual void  RotateBy(const glm::vec3 &amountRadians);
	virtual void  RotateTo(const glm::vec3 &newRotationRadions);
	glm::vec3     Scale();
	virtual void  ScaleBy(const glm::vec3 &amount);
	virtual void  ScaleTo(const glm::vec3 &newScale);
	ComponentType Type();

protected:
	void         updateMatrix();
	virtual void updateRotation();
	void         updateScale();
	void         updateTranslation();

};

#endif
