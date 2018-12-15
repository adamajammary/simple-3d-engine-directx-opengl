#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_CAMERA_H
#define GE3D_CAMERA_H

#undef far
#undef near

class Camera : public Component
{
public:
	Camera(const glm::vec3 &position, const glm::vec3 &lookAt, float fovRadians, float near, float far);
	Camera();
	~Camera() {}

private:
	float           far;
	glm::vec3       forward;
	float           fovRadians;
	float           near;
	glm::vec3       right;
	float           pitch;
	glm::mat4       projection;
	const glm::vec3 up = { 0, 1.0f, 0 };
	glm::mat4       view;
	float           yaw;

public:
	float      Far();
	bool       InputKeyboard(char key);
	void       InputMouseMove(const   wxMouseEvent &event, const MouseState &mouseState);
	void       InputMouseScroll(const wxMouseEvent &event);
	void       InvertPitch();
	void       MoveBy(const glm::vec3 &amount)      override;
	void       MoveTo(const glm::vec3 &newPosition) override;
	glm::mat4  MVP(const glm::mat4 &model, bool removeTranslation = false);
	float      Near();
	Component* Parent();
	void       RotateBy(const glm::vec3 &amountRadians)      override;
	void       RotateTo(const glm::vec3 &newRotationRadions) override;
	glm::mat4  Projection();
	void       SetFOV(const wxString &fov);
	void       UpdateProjection();
	glm::mat4  View(bool removeTranslation = false);


private:
	void init(const glm::vec3 &position, const glm::vec3 &lookAt);
	void updatePosition();
	void updateRotation() override;

};

#endif
