#include "PhysicsEngine.h"

void PhysicsEngine::CheckRayCasts(const wxMouseEvent &event)
{
	auto ray      = new RayCast(event.GetX(), event.GetY());
	auto selected = false;

	for (auto mesh : RenderEngine::Renderables)
	{
		if (mesh == nullptr)
			continue;

		auto volume = dynamic_cast<Mesh*>(mesh)->GetBoundingVolume();

		if (volume == nullptr)
			continue;

		auto min = (volume->Position() - volume->MaxBoundaries());
		auto max = (volume->Position() + volume->MaxBoundaries());

		switch (volume->VolumeType()) {
		case BOUNDING_VOLUME_BOX_AABB:
			selected = ray->RayIntersectAABB(min, max);
			break;
		case BOUNDING_VOLUME_BOX_OBB:
			selected = ray->RayIntersectAABB(min, max);
			break;
		case BOUNDING_VOLUME_SPHERE:
			selected = ray->RayIntersectSphere();
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
		for (auto child : component->Children)
		{
			if ((child != nullptr) && child->AutoRotate)
			{
				child->RotateBy(child->AutoRotation);
				dynamic_cast<Mesh*>(child)->UpdateBoundingVolume();

				if (TimeManager::TimeElapsedMS() % 100 < 20)
					RenderEngine::Canvas.Window->UpdateProperties(true);
			}
		}
	}
}
