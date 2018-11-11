#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_COMPONENT_H
#define GE3D_COMPONENT_H

class Component
{
public:
	Component(const wxString &name, const glm::vec3 &position = {}, const glm::vec3 &rotation = {}, const glm::vec3 &scale = { 1.0f, 1.0f, 1.0f }, const glm::vec4 &color = { 0.8f, 0.8f, 0.8f, 1.0f });
	Component();
	~Component();

public:
	bool                    AutoRotate;
	glm::vec3               AutoRotation;
	std::vector<Component*> Children;
	glm::vec4               Color;
	bool                    LockToParentPosition;
	bool                    LockToParentRotation;
	bool                    LockToParentScale;
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
	virtual int           GetChildIndex(Component* child);
	virtual bool          IsTextured();
	virtual bool          IsValid();
	virtual void          LoadTexture(Texture* texture, int index);
	virtual glm::mat4     Matrix();
	virtual wxString      ModelFile();
	virtual void          MoveBy(const glm::vec3 &amount);
	virtual void          MoveTo(const glm::vec3 &newPosition);
	virtual glm::vec3     Position();
	virtual int           RemoveChild(Mesh* child);
	virtual glm::vec3     Rotation();
	virtual void          RotateBy(const glm::vec3 &amountRadians);
	virtual void          RotateTo(const glm::vec3 &newRotationRadions);
	virtual glm::vec3     Scale();
	virtual void          ScaleBy(const glm::vec3 &amount);
	virtual void          ScaleTo(const glm::vec3 &newScale);
	virtual ComponentType Type();

protected:
	virtual void updateMatrix();
	virtual void updateRotation();
	virtual void updateScale();
	virtual void updateTranslation();

};

#endif
