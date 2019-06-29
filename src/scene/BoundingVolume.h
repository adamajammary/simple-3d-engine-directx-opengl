#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_BOUNDINGVOLUME_H
#define S3DE_BOUNDINGVOLUME_H

class BoundingVolume : public Mesh
{
public:
	BoundingVolume(Mesh* parent, BoundingVolumeType type, float scaleSize);
	BoundingVolume();
	~BoundingVolume() {}

private:
	BoundingVolumeType volumeType;

public:
	glm::vec3          MaxBoundaries();
	void               Update();
	BoundingVolumeType VolumeType();

private:
	void loadBoundingBox(float scaleSize);
	void loadBoundingSphere(float scaleSize);
	bool loadModelFile(aiMesh* mesh, float scaleSize);
	
};

#endif
