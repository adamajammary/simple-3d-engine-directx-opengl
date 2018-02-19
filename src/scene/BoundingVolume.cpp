#include "BoundingVolume.h"

BoundingVolume::BoundingVolume(Mesh* parent, BoundingVolumeType type, float scaleSize) : Mesh(parent, "Bounding Volume")
{
	this->volumeType = type;

	switch (type) {
		case BOUNDING_VOLUME_BOX:    this->loadBoundingBox(scaleSize);        break;
		case BOUNDING_VOLUME_SPHERE: this->loadBoundingSphere(scaleSize);     break;
		default:                     this->volumeType = BOUNDING_VOLUME_NONE; break;
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

	this->scale                = glm::vec3(scaleSize, scaleSize, scaleSize);
	this->LockToParentPosition = true;
	this->LockToParentRotation = true;
	this->LockToParentScale    = true;

	this->updateModelData();

	this->isValid = ((this->IBO() > 0) && (this->VBO() > 0));

	return this->isValid;
}

glm::vec3 BoundingVolume::MaxBoundaries()
{
	glm::vec3 parentPosition = this->Parent->Position();
	glm::vec3 parentScale    = this->Parent->Scale();
	float     maxScale       = (std::max(std::max(parentScale[0], parentScale[1]), parentScale[2]) + 0.01f);
	glm::vec3 maxBoundaries  = glm::vec3((parentPosition[0] + maxScale), (parentPosition[1] + maxScale), (parentPosition[2] + maxScale));

	return maxBoundaries;
}

glm::vec3 BoundingVolume::MinBoundaries()
{
	glm::vec3 parentPosition = this->Parent->Position();
	glm::vec3 parentScale    = this->Parent->Scale();
	float     maxScale       = (std::max(std::max(parentScale[0], parentScale[1]), parentScale[2]) + 0.01f);
	glm::vec3 minBoundaries  = glm::vec3((parentPosition[0] - maxScale), (parentPosition[1] - maxScale), (parentPosition[2] - maxScale));

	return minBoundaries;
}

BoundingVolumeType BoundingVolume::VolumeType()
{
	return this->volumeType;
}
