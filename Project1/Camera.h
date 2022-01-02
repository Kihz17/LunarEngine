#pragma once

#include <glm/glm.hpp>

enum class MoveDirection
{
	Forward,
	Right,
	Left,
	Back
};

class Camera
{
public:
	Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
	~Camera();

	glm::mat4 GetViewMatrix() const;

	void Move(MoveDirection direction, float deltaTime);
	void Look(float x, float y);

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	float yaw, pitch, speed, sensitivity, fov;

private:
	void UpdateCamera();
};