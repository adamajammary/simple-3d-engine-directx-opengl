#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_BOUNDINGVOLUME_H
#define S3DE_BOUNDINGVOLUME_H

class BoundingVolume : public Mesh
{
public:
	BoundingVolume(Mesh* parent, BoundingVolumeType type, const glm::vec3 &scaleSize);
	BoundingVolume();
	~BoundingVolume() {}

private:
	BoundingVolumeType volumeType;

public:
	glm::vec3          MaxBoundaries();
	glm::vec3          MinBoundaries();
	void               Update();
	BoundingVolumeType VolumeType();

private:
	void loadBoundingBox(const glm::vec3 &scaleSize);
	void loadBoundingSphere(const glm::vec3 &scaleSize);
	bool loadModelFile(aiMesh* mesh, const glm::vec3 &scaleSize);
	
};

#endif
