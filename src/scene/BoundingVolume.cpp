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
	this->Name    = "Bounding Box";
	auto aiMeshes = Utils::LoadModelFile(Utils::RESOURCE_MODELS[ID_ICON_CUBE]);

	if (!aiMeshes.empty()) {
		this->loadModelFile(aiMeshes[0]->Mesh, scaleSize);
		aiReleaseImport(aiMeshes[0]->Scene);
	}
	
}

void BoundingVolume::loadBoundingSphere(float scaleSize)
{
	this->Name    = "Bounding Sphere";
	auto aiMeshes = Utils::LoadModelFile(Utils::RESOURCE_MODELS[ID_ICON_ICO_SPHERE]);

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
	auto isPlane = (this->Parent->Parent->ModelFile() == Utils::RESOURCE_MODELS[ID_ICON_PLANE]);
	auto max     = this->scale;

	if (std::abs(this->rotation.x) > 0.01f)
	{
		max = glm::rotateX(max, this->rotation.x);
		max = { std::abs(max.x), std::abs(max.y), std::abs(max.z) };

		if (isPlane)
			max = { max.x, std::max(max.y, this->scale.z), std::max(max.z, this->scale.z) };
		else
			max = { max.x, std::max(std::max(max.y, max.z), this->scale.y), std::max(std::max(max.z, max.y), this->scale.z) };
	}

	if (std::abs(this->rotation.y) > 0.01f)
	{
		max = glm::rotateY(max, this->rotation.y);
		max = { std::abs(max.x), std::abs(max.y), std::abs(max.z) };

		if (isPlane)
			max = { std::max(max.x, this->scale.z), max.y, std::max(max.z, this->scale.z) };
		else
			max = { std::max(std::max(max.x, max.z), this->scale.x), max.y, std::max(std::max(max.z, max.x), this->scale.z) };
	}

	if (!isPlane)
	{
		if (std::abs(this->rotation.z) > 0.01f) {
			max = glm::rotateZ(max, this->rotation.z);
			max = { std::abs(max.x), std::abs(max.y), std::abs(max.z) };
			max = { std::max(std::max(max.x, max.y), this->scale.x), std::max(std::max(max.y, max.x), this->scale.y), max.z };
		}
	}

	return max;
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
