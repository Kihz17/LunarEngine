#pragma once

#include "RenderComponent.h"
#include "VertexArrayObject.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct RenderSubmission
{
	RenderSubmission(RenderComponent* renderComponent, const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation)
		: renderComponent(renderComponent),
		position(position),
		scale(scale),
		rotation(rotation),
		transform(1.0f)
	{
		transform *= glm::translate(glm::mat4(1.0f), position);
		transform *= glm::toMat4(rotation);
		transform *= glm::scale(glm::mat4(1.0f), scale);
	}

	RenderComponent* renderComponent;
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	glm::mat4 transform;
};

struct LineRenderSubmission
{
	VertexArrayObject* vao;
	unsigned int indexCount = 0;
	glm::vec3 lineColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float lineWidth = 1.0f;

	glm::mat4 transform = glm::mat4(1.0f);
};
