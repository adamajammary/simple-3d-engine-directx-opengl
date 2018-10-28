#include "RayCast.h"

RayCast::RayCast(int x, int y)
{
	this->direction        = this->calculateRay(x, y);
	this->inverseDirection = glm::vec3((1.0f / direction[0]), (1.0f / direction[1]), (1.0f / direction[2]));
	this->origin           = RenderEngine::Camera->Position();
}

RayCast::RayCast()
{
	this->direction        = {};
	this->inverseDirection = {};
	this->origin           = {};
}
 
glm::vec3 RayCast::calculateRay(int x, int y)
{
	// http://antongerdelan.net/opengl/raycasting.html

	// VIEWPORT SPACE
	glm::vec4 viewportSpace = { x, y, RenderEngine::Canvas.Size.GetWidth(), RenderEngine::Canvas.Size.GetHeight() };

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
		viewportSpace.y = (RenderEngine::Canvas.Size.GetHeight() - y);

	// NORMALIZED DEVICE SPACE [-1.0, 1.0]
	glm::vec2 normalizedDeviceSpace = {
		((viewportSpace[0]         / viewportSpace[2])  * 2.0f - 1.0f),
		((1.0f - (viewportSpace[1] / viewportSpace[3])) * 2.0f - 1.0f)
	};
            
	// (HOMOGENEOUS) CLIP SPACE
	glm::vec4 clipSpace = glm::vec4(normalizedDeviceSpace[0], normalizedDeviceSpace[1], -1.0f, 1.0f);
            
	// EYE (CAMERA) SPACE
	glm::mat4 inverseProjectionMatrix = glm::inverse(RenderEngine::Camera->Projection());
	glm::vec4 eyeSpace                = (inverseProjectionMatrix * clipSpace);

	eyeSpace[2] = -1.0f;
	eyeSpace[3] = 0.0f;
            
	// WORLD SPACE
	glm::mat4 inverseViewMatrix = glm::inverse(RenderEngine::Camera->View());
	glm::vec4 worldSpace        = (inverseViewMatrix * eyeSpace);

	// RAY
	glm::vec3 ray = glm::normalize(glm::vec3(worldSpace[0], worldSpace[1], worldSpace[2]));

	return ray;
}

bool RayCast::RayIntersectAABB(const glm::vec3 &boxMin, const glm::vec3 &boxMax)
{
	// https://www.unknowncheats.me/forum/counterstrike-global-offensive/136361-external-ray-tracing-ray-aabb.html
	// https://www.unknowncheats.me/forum/counterstrike-source/109498-efficient-iscrossonhitbox-algorithm.html
	
	// If line is parallel and outside the box it is not possible to intersect

	// X
	if ((this->direction[0] == 0.0f) && ((this->origin[0] < std::min(boxMin[0], boxMax[0])) || (this->origin[0] > std::max(boxMin[0], boxMax[0]))))
		return false;
    // Y
	if ((this->direction[1] == 0.0f) && ((this->origin[1] < std::min(boxMin[1], boxMax[1])) || (this->origin[1] > std::max(boxMin[1], boxMax[1]))))
		return false;
	// Z
	if ((this->direction[2] == 0.0f) && ((this->origin[2] < std::min(boxMin[2], boxMax[2])) || (this->origin[2] > std::max(boxMin[2], boxMax[2]))))
		return false;

	// 6 FACES / SIDES / PLANES
	float t1 = ((boxMin[0] - this->origin[0]) * this->inverseDirection[0]);   // LEFT
	float t2 = ((boxMax[0] - this->origin[0]) * this->inverseDirection[0]);   // RIGHT
	float t3 = ((boxMin[1] - this->origin[1]) * this->inverseDirection[1]);   // BOTTON
	float t4 = ((boxMax[1] - this->origin[1]) * this->inverseDirection[1]);   // TOP
	float t5 = ((boxMin[2] - this->origin[2]) * this->inverseDirection[2]);   // BACK
	float t6 = ((boxMax[2] - this->origin[2]) * this->inverseDirection[2]);   // FRONT
        
	float tMin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	float tMax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
        
	if ((tMax < 0) || (tMin > tMax))
		return false;

	return true;
}

bool RayCast::RayIntersectSphere(const glm::vec3 &boxMin, const glm::vec3 &boxMax)
{
	RenderEngine::Canvas.Window->SetStatusText("RayIntersectSphere: NOT IMPLEMENTED");
	return false;
}
