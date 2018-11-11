#include "PhysicsEngine.h"

void PhysicsEngine::CheckRayCasts(wxMouseEvent &event)
{
	BoundingVolume* volume;
	RayCast*        ray = new RayCast(event.GetX(), event.GetY());

	for (auto mesh : RenderEngine::Renderables)
	{
		if (mesh == nullptr)
			continue;

		volume = dynamic_cast<Mesh*>(mesh)->GetBoundingVolume();

		if (volume == nullptr)
			continue;

		switch (volume->VolumeType()) {
		case BOUNDING_VOLUME_BOX:
			dynamic_cast<Mesh*>(mesh)->Select(ray->RayIntersectAABB(volume->MinBoundaries(), volume->MaxBoundaries()));
			break;
		case BOUNDING_VOLUME_SPHERE:
			dynamic_cast<Mesh*>(mesh)->Select(ray->RayIntersectSphere(volume->MinBoundaries(), volume->MaxBoundaries()));
			break;
		}
	}
}

void PhysicsEngine::Update()
{
	for (auto component : SceneManager::Components)
	{
		for (auto child : component->Children) {
			if ((child != nullptr) && child->AutoRotate)
				child->RotateBy(child->AutoRotation);
		}
	}
}
