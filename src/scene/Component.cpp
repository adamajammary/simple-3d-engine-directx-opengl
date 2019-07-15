#include "Component.h"

uint32_t Component::sid = 0;

Component::Component(const wxString &name, const glm::vec3 &position)
{
	this->AutoRotate        = false;
	this->AutoRotation      = {};
	this->isValid           = false;
	//this->LockToParentPosition = false;
	//this->LockToParentRotation = false;
	//this->LockToParentScale    = false;
	this->ComponentMaterial = {};
	this->modelFile         = "";
	this->Name              = name;
	this->Parent            = nullptr;
	this->position          = position;
	this->rotation          = {};
	this->scale             = { 1.0f, 1.0f, 1.0f };
	this->type              = COMPONENT_UNKNOWN;

	for (uint32_t i = 0; i < MAX_TEXTURES; i++)
		this->Textures[i] = nullptr;

	this->id = Component::sid++;
}

Component::~Component()
{
	if (this->type != COMPONENT_CAMERA) {
		for (auto child : this->Children)
			_DELETEP(child);
	}

	this->Children.clear();

	for (uint32_t i = 0; i < MAX_TEXTURES; i++) {
		if ((this->type != COMPONENT_WATER) && (this->Textures[i] != SceneManager::EmptyTexture) && (this->Textures[i] != SceneManager::EmptyCubemap)) {
			_DELETEP(this->Textures[i]);
		}
	}
}

int Component::GetChildIndex(Component* child)
{
	for (int i = 0; i < (int)this->Children.size(); i++) {
		if (this->Children[i]->ID() == child->ID())
			return i;
	}

	return -1;
}

uint32_t Component::ID()
{
	return this->id;
}

bool Component::IsTextured(int index)
{
	if ((index < 0) || (index > MAX_TEXTURES))
		return false;

	Texture* texture = this->Textures[index];

	if (texture == nullptr)
		return false;

	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		return ((texture->SRV11 != nullptr) && !texture->ImageFile().empty());
	case GRAPHICS_API_DIRECTX12:
		return ((texture->Resource12 != nullptr) && !texture->ImageFile().empty());
	#endif
	case GRAPHICS_API_OPENGL:
		return ((texture->ID() > 0) && !texture->ImageFile().empty());
	case GRAPHICS_API_VULKAN:
		return ((texture->ImageView != nullptr) && (texture->Sampler != nullptr) && !texture->ImageFile().empty());
	default:
		throw;
	}

	return false;
}
bool Component::IsValid()
{
	return this->isValid;
}

void Component::LoadTexture(Texture* texture, int index)
{
    this->Textures[index] = texture;
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

void Component::RotateTo(const glm::vec3 &newRotationRadians)
{
	this->rotation = newRotationRadians;
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
	float fullRotation = (2.0f * PI);

	for (int i = 0; i < 3; i++)
	{
		if (this->rotation[i] > fullRotation)
			this->rotation[i] -= fullRotation;
		else if (this->rotation[i] < -fullRotation)
			this->rotation[i] += fullRotation;
	}

	//if (this->LockToParentRotation && (this->Parent != nullptr)) {
	//	this->rotation[0] += this->Parent->rotation[0];
	//	this->rotation[1] += this->Parent->rotation[1];
	//	this->rotation[2] += this->Parent->rotation[2];
	//}

	glm::mat4 rotateMatrixX = glm::rotate(this->rotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotateMatrixY = glm::rotate(this->rotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotateMatrixZ = glm::rotate(this->rotation[2], glm::vec3(0.0f, 0.0f, 1.0f));

	this->rotationMatrix = (rotateMatrixZ * rotateMatrixY * rotateMatrixX);

	this->updateMatrix();
}

void Component::updateScale()
{
	glm::vec3 scaleVector;
	
	//if (this->LockToParentScale && (this->Parent != nullptr))
	//	scaleVector = (this->scale * this->Parent->scale);
	//else
    scaleVector = this->scale;

	this->scaleMatrix = glm::scale(scaleVector);

	this->updateMatrix();
}

void Component::updateTranslation()
{
	glm::vec3 positionVector;

    //if (this->LockToParentPosition && (this->Parent != nullptr))
    //    positionVector = (this->position + this->Parent->position);
    //else
    positionVector = this->position;

	this->translationMatrix = glm::translate(positionVector);

	this->updateMatrix();
}
