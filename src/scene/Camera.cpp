#include "Camera.h"

Camera::Camera(const glm::vec3 &position, const glm::vec3 &lookAt, float fovRadians, float near, float far)
	: Component("Camera", position)
{
	this->far        = far;
	this->fovRadians = fovRadians;
	this->near       = near;
	this->pitch      = 0.0f;
	this->yaw        = -(glm::pi<float>() / 2.0f);
	this->type       = COMPONENT_CAMERA;

	this->init(position, lookAt);

	this->isValid = true;
}

Camera::Camera()
{
	this->far        = 0.0f;
	this->fovRadians = 0.0f;
	this->near       = 0.0f;
	this->pitch      = 0.0f;
	this->yaw        = 0.0f;
	this->type       = COMPONENT_CAMERA;
}

float Camera::Far()
{
	return far;
}

void Camera::init(const glm::vec3 &position, const glm::vec3 &lookAt)
{
	this->forward = glm::normalize(lookAt - position);
	this->right   = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), this->forward));

	this->UpdateProjection();
	this->updatePosition();
	this->updateRotation();
}

bool Camera::InputKeyboard(char key)
{
	glm::vec3 moveVector;
	float     moveModifier = (TimeManager::DeltaTime * 20.0);
	glm::vec3 moveAmount   = glm::vec3(moveModifier, moveModifier, moveModifier);
	bool      result       = false;

	switch (toupper(key)) {
	case 'W':
		moveVector = (this->forward * moveAmount);
		result     = true;
		break;
	case 'A':
		moveVector  = glm::normalize(glm::cross(this->forward, glm::vec3(0.0, 1.0, 0.0)));
		moveVector *= -moveAmount;
		result      = true;
		break;
	case 'S':
		moveVector = (this->forward * -moveAmount);
		result     = true;
		break;
	case 'D':
		moveVector  = glm::normalize(glm::cross(this->forward, glm::vec3(0.0, 1.0, 0.0)));
		moveVector *= moveAmount;
		result      = true;
		break;
	}

	if (result)
		this->MoveBy(moveVector);

	return result;
}

void Camera::InputMouseMove(const wxMouseEvent &event, const MouseState &mouseState)
{
	glm::vec3 moveVector;
	glm::vec2 mouseMovement = glm::vec2((event.GetX() - mouseState.Position.x), (event.GetY() - mouseState.Position.y));
	glm::vec2 moveModifier  = glm::vec2((mouseMovement.x * TimeManager::DeltaTime * 3.0), (mouseMovement.y * TimeManager::DeltaTime * 3.0));
	glm::vec3 moveAmountX   = glm::vec3(-moveModifier.x, -moveModifier.x, -moveModifier.x);
	glm::vec3 moveAmountY   = glm::vec3(-moveModifier.y, -moveModifier.y, -moveModifier.y);

	// MOVE/PAN HORIZONTAL/VERTICAL
	if (event.GetModifiers() == wxMOD_SHIFT)
	{
		moveVector  = glm::normalize(glm::cross(this->forward, glm::vec3(0.0, 1.0, 0.0)));
		moveVector *= moveAmountX;

		this->MoveBy(moveVector);
		this->MoveBy(glm::vec3(0.0, moveModifier.y, 0.0));
	// MOVE/PAN FORWARD/BACK (Z)
	} else if (event.GetModifiers() == wxMOD_CONTROL) {
		moveVector = (this->forward * moveAmountY);
		this->MoveBy(moveVector);
	// ROTATE HORIZONTAL/VERTICAL (YAW/PITCH)
	} else {
		this->RotateBy(glm::vec3(-(moveModifier.y * 0.01), (moveModifier.x * 0.01), 0.0));
	}
}

void Camera::InputMouseScroll(const wxMouseEvent &event)
{
	glm::vec3 moveVector;
	float     moveModifier = (event.GetWheelRotation() * TimeManager::DeltaTime * 1.0);
    glm::vec3 moveAmount   = glm::vec3(moveModifier, moveModifier, moveModifier);

    // UP / DOWN (Y)
	if (event.GetModifiers() == wxMOD_SHIFT) {
		this->MoveBy(glm::vec3(0.0, (event.GetWheelRotation() * TimeManager::DeltaTime * 1.0), 0.0));
    // LEFT / RIGHT (X)
	} else if (event.GetModifiers() == wxMOD_CONTROL) {
		moveVector  = glm::normalize(glm::cross(this->forward, glm::vec3(0.0, 1.0, 0.0)));
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
	this->RotateTo(glm::vec3(this->pitch, this->yaw, 0.0f));
}

bool Camera::IsRenderable()
{
	return false;
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

void Camera::RotateTo(const glm::vec3 &newRotationRadions)
{
	this->pitch = newRotationRadions.x;
	this->yaw   = newRotationRadions.y;
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
}

void Camera::updatePosition()
{
	this->center = (this->position + this->forward);
	this->view   = glm::lookAt(this->position, this->center, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::updateRotation()
{
	// https://learnopengl.com/#!Getting-started/Camera
	this->pitch    = std::max(std::min(this->pitch, (glm::pi<float>() / 2.0f)), -(glm::pi<float>() / 2.0f));
	this->rotation = glm::vec3(this->pitch, this->yaw, 0.0f);

	this->center.x = (std::cos(this->pitch) * std::cos(this->yaw));
	this->center.y = std::sin(this->pitch);
	this->center.z = (std::cos(this->pitch) * std::sin(this->yaw));

	this->forward  = glm::normalize(this->center);

	this->center = (this->position + this->forward);
	this->view   = glm::lookAt(this->position, this->center, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::View(bool removeTranslation)
{
	return (removeTranslation ? glm::mat4(glm::mat3(this->view)) : this->view);
}
