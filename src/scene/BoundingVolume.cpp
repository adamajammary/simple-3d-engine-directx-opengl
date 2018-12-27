#include "BoundingVolume.h"

BoundingVolume::BoundingVolume(Mesh* parent, BoundingVolumeType type, float scaleSize) : Mesh(parent, "Bounding Volume")
{
	this->volumeType = type;

	switch (type) {
		case BOUNDING_VOLUME_BOX:    this->loadBoundingBox(scaleSize);        break;
		case BOUNDING_VOLUME_SPHERE: this->loadBoundingSphere(scaleSize);     break;
		default: throw;
	}
}

BoundingVolume::BoundingVolume() : Mesh(nullptr, "")
{
	this->volumeType = BOUNDING_VOLUME_NONE;
}

void BoundingVolume::loadBoundingBox(float scaleSize)
{
	this->Name = "Bounding Box";
	std::vector<AssImpMesh*> aiMeshes = Utils::LoadModelFile(Utils::RESOURCE_MODELS[ID_ICON_CUBE]);

	if (!aiMeshes.empty()) {
		this->loadModelFile(aiMeshes[0]->Mesh, scaleSize);
		aiReleaseImport(aiMeshes[0]->Scene);
	}
	
}

void BoundingVolume::loadBoundingSphere(float scaleSize)
{
	this->Name = "Bounding Sphere";
	std::vector<AssImpMesh*> aiMeshes = Utils::LoadModelFile(Utils::RESOURCE_MODELS[ID_ICON_ICO_SPHERE]);

	if (!aiMeshes.empty()) {
		this->loadModelFile(aiMeshes[0]->Mesh, scaleSize);
		aiReleaseImport(aiMeshes[0]->Scene);
	}
}

bool BoundingVolume::loadModelFile(aiMesh* mesh, float scaleSize)
{
	if (mesh == nullptr)
		return false;

	this->loadModelData(mesh);
	this->setModelData();

	this->scale = glm::vec3(scaleSize, scaleSize, scaleSize);
	//this->LockToParentPosition = true;
	//this->LockToParentRotation = true;
	//this->LockToParentScale    = true;

	this->updateModelData();

	this->isValid = ((this->IBO() > 0) && (this->VBO() > 0));

	return this->isValid;
}

glm::vec3 BoundingVolume::MaxBoundaries()
{
	glm::vec3 max = (this->position + this->scale);

	max = glm::rotateX(max, this->rotation.x);
	max = glm::rotateY(max, this->rotation.y);
	max = glm::rotateZ(max, this->rotation.z);

	return { std::abs(max.x), std::abs(max.y), std::abs(max.z) };
}

glm::vec3 BoundingVolume::MinBoundaries()
{
	glm::vec3 min = (this->position - this->scale);

	min = glm::rotateX(min, this->rotation.x);
	min = glm::rotateY(min, this->rotation.y);
	min = glm::rotateZ(min, this->rotation.z);

	return { -std::abs(min.x), -std::abs(min.y), -std::abs(min.z) };
}

void BoundingVolume::Update()
{
	// Align with parent
	this->MoveTo(this->Parent->Position());
	this->ScaleTo(this->Parent->Scale());
	this->RotateTo(this->Parent->Rotation());

	// Expand the bounding volume to fit the entire rotated mesh
	this->ScaleTo(this->MaxBoundaries());

	// Reset rotation of the bounding volume
	this->RotateTo({});
}

BoundingVolumeType BoundingVolume::VolumeType()
{
	return this->volumeType;
}
