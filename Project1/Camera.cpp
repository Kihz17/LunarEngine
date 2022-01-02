#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
	: position(position),
	speed(4.0f),
	sensitivity(0.1f),
	yaw(yaw),
	pitch(pitch),
	up(up)
{
	UpdateCamera();
}

Camera::~Camera()
{

}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(this->position, this->position + this->front, this->up);
}

void Camera::UpdateCamera()
{
	glm::vec3 front;
	front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	front.y = sin(glm::radians(this->pitch));
	front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

	this->front = glm::normalize(front);
	this->right = glm::normalize(glm::cross(this->front, this->up));
	this->up = glm::normalize(glm::cross(this->right, this->front));
}

void Camera::Move(MoveDirection direction, float deltaTime)
{
	float scalar = speed * deltaTime;

	if (direction == MoveDirection::Forward)
	{
		position += front * scalar;
	}
	else if (direction == MoveDirection::Right)
	{
		position += right * scalar;
	}
	else if (direction == MoveDirection::Left)
	{
		position -= right * scalar;
	}
	else if (direction == MoveDirection::Back)
	{
		position -= front * scalar;
	}
}

void Camera::Look(float x, float y)
{
	x *= sensitivity;
	y *= sensitivity;

	yaw += x;
	pitch += y;

	if (pitch > 89.0f)
		pitch = 89.0f;

	if (pitch < -89.0f)
		pitch = 89.0f;

	UpdateCamera();
}