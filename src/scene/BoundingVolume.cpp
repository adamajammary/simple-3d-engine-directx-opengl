#include "BoundingVolume.h"

BoundingVolume::BoundingVolume(Mesh* parent, BoundingVolumeType type, const glm::vec3 &scaleSize) : Mesh(parent, "Bounding Volume")
{
	this->volumeType = type;

	switch (type) {
	case BOUNDING_VOLUME_BOX_AABB:
	case BOUNDING_VOLUME_BOX_OBB:
		this->loadBoundingBox(scaleSize);
		break;
	case BOUNDING_VOLUME_SPHERE:
		this->loadBoundingSphere(scaleSize);
		break;
	default:
		throw;
	}
}

BoundingVolume::BoundingVolume() : Mesh(nullptr, "")
{
	this->volumeType = BOUNDING_VOLUME_NONE;
}

void BoundingVolume::loadBoundingBox(const glm::vec3 &scaleSize)
{
	this->Name    = "Bounding Box";
	auto aiMeshes = Utils::LoadModelFile(Utils::RESOURCE_MODELS[ID_ICON_CUBE]);

	if (!aiMeshes.empty()) {
		this->loadModelFile(aiMeshes[0]->Mesh, scaleSize);
		aiReleaseImport(aiMeshes[0]->Scene);
	}
	
}

void BoundingVolume::loadBoundingSphere(const glm::vec3 &scaleSize)
{
	this->Name    = "Bounding Sphere";
	auto aiMeshes = Utils::LoadModelFile(Utils::RESOURCE_MODELS[ID_ICON_ICO_SPHERE]);

	if (!aiMeshes.empty()) {
		this->loadModelFile(aiMeshes[0]->Mesh, scaleSize);
		aiReleaseImport(aiMeshes[0]->Scene);
	}
}

bool BoundingVolume::loadModelFile(aiMesh* mesh, const glm::vec3 &scaleSize)
{
	if (mesh == nullptr)
		return false;

	this->loadModelData(mesh);
	this->setModelData();

	this->scale = glm::vec3((scaleSize.x + 0.01), (scaleSize.y + 0.01), (scaleSize.z + 0.01));

	this->updateModelData();

	this->isValid = ((this->IBO() > 0) && (this->VBO() > 0));

	return this->isValid;
}

glm::vec3 BoundingVolume::MaxBoundaries()
{
	glm::vec3 max = dynamic_cast<Mesh*>(this->Parent)->MaxVertexPosition();
	return glm::vec3((this->position.x + max.x), (this->position.y + max.y), (this->position.z - max.z));
}

glm::vec3 BoundingVolume::MinBoundaries()
{
	glm::vec3 max = dynamic_cast<Mesh*>(this->Parent)->MaxVertexPosition();
	return glm::vec3((this->position.x - max.x), (this->position.y - max.y), (this->position.z + max.z));
}

void BoundingVolume::Update()
{
	this->MoveTo(this->Parent->Position());
	this->ScaleTo(dynamic_cast<Mesh*>(this->Parent)->MaxVertexPosition(true));
}

BoundingVolumeType BoundingVolume::VolumeType()
{
	return this->volumeType;
}
