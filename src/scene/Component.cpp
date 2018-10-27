#include "Component.h"

Component::Component(const wxString &name, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const glm::vec4 &color)
{
	this->AutoRotate           = false;
	this->AutoRotation         = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Color                = color;
	this->isValid              = false;
	this->modelFile            = "";
	this->LockToParentPosition = false;
	this->LockToParentRotation = false;
	this->LockToParentScale    = false;
	this->Name                 = name;
	this->Parent               = nullptr;
	this->position             = position;
	this->rotation             = rotation;
	this->scale                = scale;
	this->type                 = COMPONENT_UNKNOWN;

	for (int i = 0; i < MAX_TEXTURES; i++)
		this->Textures[i] = nullptr;
}

Component::Component()
{
	this->AutoRotate           = false;
	this->AutoRotation         = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Color                = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	this->isValid              = false;
	this->modelFile            = "";
	this->LockToParentPosition = false;
	this->LockToParentRotation = false;
	this->LockToParentScale    = false;
	this->Name                 = "";
	this->Parent               = nullptr;
	this->position             = glm::vec3(0.0f, 0.0f, 0.0f);
	this->rotation             = glm::vec3(0.0f, 0.0f, 0.0f);
	this->scale                = glm::vec3(0.0f, 0.0f, 0.0f);
	this->type                 = COMPONENT_UNKNOWN;

	for (int i = 0; i < MAX_TEXTURES; i++)
		this->Textures[i] = nullptr;
}

Component::~Component()
{
	for (auto child : this->Children)
		_DELETEP(child);

	this->Children.clear();

	for (int i = 0; i < MAX_TEXTURES; i++) {
		if ((this->Textures[i] == Utils::EmptyTexture) || (this->Textures[i] == Utils::EmptyCubemap))
			continue;

		_DELETEP(this->Textures[i]);
	}
}

int Component::GetChildIndex(Mesh* child)
{
	for (int i = 0; i < (int)this->Children.size(); i++) {
		if (this->Children[i] == child)
			return i;
	}

	return -1;
}

bool Component::IsTextured()
{
	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		return ((this->Textures[0] != nullptr) && (this->Textures[0]->SRV11 != nullptr) && !this->Textures[0]->ImageFile().empty());
	case GRAPHICS_API_DIRECTX12:
		return ((this->Textures[0] != nullptr) && (this->Textures[0]->Resource12 != nullptr) && !this->Textures[0]->ImageFile().empty());
	#endif
	case GRAPHICS_API_OPENGL:
		return ((this->Textures[0] != nullptr) && (this->Textures[0]->ID() > 0) && !this->Textures[0]->ImageFile().empty());
	case GRAPHICS_API_VULKAN:
		return ((this->Textures[0] != nullptr) && (this->Textures[0]->ImageView != nullptr) && (this->Textures[0]->Sampler != nullptr) && !this->Textures[0]->ImageFile().empty());
	}

	return false;
}
bool Component::IsValid()
{
	return this->isValid;
}

glm::mat4 Component::Matrix()
{
	return this->matrix;
}

wxString Component::ModelFile()
{
	return this->modelFile;
}

void Component::MoveBy(const glm::vec3 &amount)
{
	this->position += amount;
	this->updateTranslation();
}

void Component::MoveTo(const glm::vec3 &newPosition)
{
	this->position = newPosition;
	this->updateTranslation();
}

glm::vec3 Component::Position()
{
	return this->position;
}

int Component::RemoveChild(Mesh* child)
{
    int index = this->GetChildIndex(child);

    if (index < 0)
		return -1;

	this->Children.erase(this->Children.begin() + index);
            
    return 0;
}

glm::vec3 Component::Rotation()
{
	return this->rotation;
}

void Component::RotateBy(const glm::vec3 &amountRadians)
{
	this->rotation += amountRadians;
	this->updateRotation();
}

void Component::RotateTo(const glm::vec3 &newRotationRadions)
{
	this->rotation = newRotationRadions;
	this->updateRotation();
}

glm::vec3 Component::Scale()
{
	return this->scale;
}

void Component::ScaleBy(const glm::vec3 &amount)
{
	this->scale += amount;
	this->updateScale();
}

void Component::ScaleTo(const glm::vec3 &newScale)
{
	this->scale = newScale;
	this->updateScale();
}

ComponentType Component::Type()
{
	return this->type;
}

void Component::updateMatrix()
{
	this->matrix = (this->translationMatrix * this->rotationMatrix * this->scaleMatrix);
}

void Component::updateRotation()
{
	// RESET ROTATION AFTER 360 DEGREES (2PI)
	for (int i = 0; i < 3; i++)
	{
		if (this->rotation[i] > 2.0f * glm::pi<float>())
			this->rotation[i] -= (2.0f * glm::pi<float>());
		else if (this->rotation[i] < -2.0f * glm::pi<float>())
			this->rotation[i] += (2.0f * glm::pi<float>());
	}

	if (this->LockToParentRotation && (this->Parent != nullptr)) {
		this->rotation[0] += this->Parent->rotation[0];
		this->rotation[1] += this->Parent->rotation[1];
		this->rotation[2] += this->Parent->rotation[2];
	}

	glm::mat4 rotateMatrixX = glm::rotate(this->rotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotateMatrixY = glm::rotate(this->rotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotateMatrixZ = glm::rotate(this->rotation[2], glm::vec3(0.0f, 0.0f, 1.0f));

	this->rotationMatrix = (rotateMatrixZ * rotateMatrixY * rotateMatrixX);

	this->updateMatrix();
}

void Component::updateScale()
{
	glm::vec3 scaleVector;
	
	if (this->LockToParentScale && (this->Parent != nullptr))
		scaleVector = (this->scale * this->Parent->scale);
    else
        scaleVector = this->scale;

	this->scaleMatrix = glm::scale(scaleVector);

	this->updateMatrix();
}

void Component::updateTranslation()
{
	glm::vec3 positionVector;

    if (this->LockToParentPosition && (this->Parent != nullptr))
        positionVector = (this->position + this->Parent->position);
    else
        positionVector = this->position;

	this->translationMatrix = glm::translate(positionVector);

	this->updateMatrix();
}
