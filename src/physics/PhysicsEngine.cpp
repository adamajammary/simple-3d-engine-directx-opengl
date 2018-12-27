#include "PhysicsEngine.h"

void PhysicsEngine::CheckRayCasts(wxMouseEvent &event)
{
	RayCast* ray      = new RayCast(event.GetX(), event.GetY());
	bool     selected = false;

	for (auto mesh : RenderEngine::Renderables)
	{
		if (mesh == nullptr)
			continue;

		BoundingVolume* volume = dynamic_cast<Mesh*>(mesh)->GetBoundingVolume();

		if (volume == nullptr)
			continue;

		switch (volume->VolumeType()) {
		case BOUNDING_VOLUME_BOX:
			selected = ray->RayIntersectAABB(volume->MinBoundaries(), volume->MaxBoundaries());
			break;
		case BOUNDING_VOLUME_SPHERE:
			selected = ray->RayIntersectSphere(volume->MinBoundaries(), volume->MaxBoundaries());
			break;
		default:
			throw;
		}

		dynamic_cast<Mesh*>(mesh)->Select(selected);
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
