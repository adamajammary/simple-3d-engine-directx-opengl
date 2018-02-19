#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_BOUNDINGVOLUME_H
#define GE3D_BOUNDINGVOLUME_H

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
	glm::vec3          MinBoundaries();
	BoundingVolumeType VolumeType();

private:
	void loadBoundingBox(float scaleSize);
	void loadBoundingSphere(float scaleSize);
	bool loadModelFile(aiMesh* mesh, float scaleSize);
	
};

#endif
