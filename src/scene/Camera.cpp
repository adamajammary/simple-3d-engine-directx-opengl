#include "Camera.h"

Camera::Camera(const glm::vec3 &position, const glm::vec3 &lookAt, float fovRadians, float near, float far)
	: Component("Camera", position)
{
	this->fovRadians = fovRadians;
	this->near       = near;
	this->far        = far;
	this->pitch      = 0;
	this->yaw        = -(glm::pi<float>() * 0.5f);
	this->type       = COMPONENT_CAMERA;
	this->isValid    = true;

	this->init(position, lookAt);

	this->Children = { this };
}

Camera::Camera() : Component("Camera", { 0.0f, 2.5f, 10.0f })
{
	this->fovRadians = (glm::pi<float>() * 0.25f);
	this->near       = 0.1f;
	this->far        = 100.0f;
	this->pitch      = 0;
	this->yaw        = -(glm::pi<float>() * 0.5f);
	this->type       = COMPONENT_CAMERA;
	this->isValid    = true;

	this->init(this->position, {});

	this->Children = { this };
}

void Camera::init(const glm::vec3 &position, const glm::vec3 &lookAt)
{
	this->forward = glm::normalize(lookAt - position);
	this->right   = glm::normalize(glm::cross(this->up, this->forward));

	this->UpdateProjection();
	this->updatePosition();
	this->updateRotation();
}

float Camera::Far()
{
	return far;
}

bool Camera::InputKeyboard(char key)
{
	glm::vec3    moveVector;
	const double MOVE_SPEED   = 20.0;
	float        moveModifier = (TimeManager::DeltaTime * MOVE_SPEED);
	glm::vec3    moveAmount   = { moveModifier, moveModifier, moveModifier };
	bool         result       = false;

	switch (toupper(key)) {
	case 'W':
		moveVector = (this->forward * moveAmount);
		result     = true;
		break;
	case 'A':
		moveVector  = glm::normalize(glm::cross(this->forward, this->up));
		moveVector *= -moveAmount;
		result      = true;
		break;
	case 'S':
		moveVector = (this->forward * -moveAmount);
		result     = true;
		break;
	case 'D':
		moveVector  = glm::normalize(glm::cross(this->forward, this->up));
		moveVector *= moveAmount;
		result      = true;
		break;
	default:
		break;
	}

	if (result)
		this->MoveBy(moveVector);

	return result;
}

void Camera::InputMouseMove(const wxMouseEvent &event, const MouseState &mouseState)
{
	glm::vec3    moveVector;
	const double MOVE_SPEED    = 3.0;
	glm::vec2    mouseMovement = { (event.GetX() - mouseState.Position.x), (event.GetY() - mouseState.Position.y) };
	glm::vec2    moveModifier  = { (mouseMovement.x * TimeManager::DeltaTime * MOVE_SPEED), (mouseMovement.y * TimeManager::DeltaTime * MOVE_SPEED) };
	glm::vec3    moveAmountX   = { -moveModifier.x, -moveModifier.x, -moveModifier.x };
	glm::vec3    moveAmountY   = { -moveModifier.y, -moveModifier.y, -moveModifier.y };

	// MOVE/PAN HORIZONTAL/VERTICAL
	if (event.GetModifiers() == wxMOD_SHIFT)
	{
		moveVector  = glm::normalize(glm::cross(this->forward, this->up));
		moveVector *= moveAmountX;

		this->MoveBy(moveVector);
		this->MoveBy({ 0, moveModifier.y, 0 });
	// MOVE/PAN FORWARD/BACK (Z)
	} else if (event.GetModifiers() == wxMOD_CONTROL) {
		this->MoveBy(this->forward * moveAmountY);
	// ROTATE HORIZONTAL/VERTICAL (YAW/PITCH)
	} else {
		this->RotateBy({ -(moveModifier.y * 0.01f), (moveModifier.x * 0.01f), 0 });
	}
}

void Camera::InputMouseScroll(const wxMouseEvent &event)
{
	glm::vec3    moveVector;
	const double MOVE_SPEED   = 20.0;
	float        moveModifier = ((std::signbit((float)event.GetWheelRotation()) ? -1.0 : 1.0) * TimeManager::DeltaTime * MOVE_SPEED);
	glm::vec3    moveAmount = { moveModifier, moveModifier, moveModifier };

    // UP / DOWN (Y)
	if (event.GetModifiers() == wxMOD_SHIFT) {
		moveVector = { 0, moveModifier, 0 };
    // LEFT / RIGHT (X)
	} else if (event.GetModifiers() == wxMOD_CONTROL) {
		moveVector  = glm::normalize(glm::cross(this->forward, this->up));
		moveVector *= moveAmount;
    // FORWARD / BACK (Z)
    } else {
		moveVector = (this->forward * moveAmount);
    }

    this->MoveBy(moveVector);
}

void Camera::InvertPitch()
{
	this->pitch = -this->pitch;
	this->RotateTo({ this->pitch, this->yaw, 0 });
}

void Camera::MoveBy(const glm::vec3 &amount)
{
	this->position += amount;
	this->updatePosition();
}

void Camera::MoveTo(const glm::vec3 &newPosition)
{
	this->position = newPosition;
	this->updatePosition();
}

glm::mat4 Camera::MVP(const glm::mat4 &model, bool removeTranslation)
{
	return (this->projection * this->View(removeTranslation) * model);
}

float Camera::Near()
{
	return this->near;
}

Component* Camera::Parent()
{
	return nullptr;
}

void Camera::RotateBy(const glm::vec3 &amountRadians)
{
	this->pitch += amountRadians.x;
	this->yaw   += amountRadians.y;

	this->updateRotation();
}

void Camera::RotateTo(const glm::vec3 &newRotationRadians)
{
	this->pitch = newRotationRadians.x;
	this->yaw   = newRotationRadians.y;

	this->updateRotation();
}

glm::mat4 Camera::Projection()
{
	return projection;
}

void Camera::SetFOV(const wxString &fov)
{
	this->fovRadians = Utils::ToRadians((float)std::atof(fov.c_str()));
	this->UpdateProjection();
}

void Camera::UpdateProjection()
{
	float aspectRatio = (float)((float)RenderEngine::Canvas.Size.GetWidth() / (float)RenderEngine::Canvas.Size.GetHeight());
	this->projection  = glm::perspective(this->fovRadians, aspectRatio, this->near, this->far);

	// Flip Y-axis on Vulkan
	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
		this->projection[1][1] *= -1;
}

void Camera::updatePosition()
{
	glm::vec3 center = (this->position + this->forward);
	this->view       = glm::lookAt(this->position, center, this->up);
}

void Camera::updateRotation()
{
	// https://learnopengl.com/#!Getting-started/Camera
	this->pitch    = std::max(std::min(this->pitch, (glm::pi<float>() * 0.5f)), -(glm::pi<float>() * 0.5f));
	this->rotation = { this->pitch, this->yaw, 0 };

	glm::vec3 center = {
		(std::cos(this->pitch) * std::cos(this->yaw)),	// X
		std::sin(this->pitch),							// Y
		(std::cos(this->pitch) * std::sin(this->yaw))	// Z
	};

	this->forward = glm::normalize(center);

	center     = (this->position + this->forward);
	this->view = glm::lookAt(this->position, center, this->up);
}

glm::mat4 Camera::View(bool removeTranslation)
{
	return (removeTranslation ? glm::mat4(glm::mat3(this->view)) : this->view);
}
