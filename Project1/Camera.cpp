#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
	: position(position),
	speed(8.0f),
	sensitivity(0.1f),
	yaw(yaw),
	pitch(pitch),
	worldUp(up),
	fov(glm::radians(45.0f)),
	first(true),
	lastX(0.0f),
	lastY(0.0f)
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
	this->right = glm::normalize(glm::cross(this->front, this->worldUp));
	this->up = glm::normalize(glm::cross(this->right, this->front));
}

void Camera::Move(MoveDirection direction, float deltaTime)
{
	float scalar = speed * deltaTime;

	if (direction == MoveDirection::Forward)
	{
		position += front * scalar;
	}
	else if (direction == MoveDirection::Back)
	{
		position -= front * scalar;
	}

	if (direction == MoveDirection::Right)
	{
		position += right * scalar;
	}
	else if (direction == MoveDirection::Left)
	{
		position -= right * scalar;
	}
	
	if (direction == MoveDirection::Up)
	{
		position.y += scalar;
	}
	else if (direction == MoveDirection::Down)
	{
		position.y -= scalar;
	}
}

void Camera::Look(float xPos, float yPos)
{
	if (first)
	{
		first = false;
		lastX = xPos;
		lastY = yPos;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos;
	lastX = xPos;
	lastY = yPos;

	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;

	if (pitch < -89.0f)
		pitch = -89.0f;

	UpdateCamera();
}