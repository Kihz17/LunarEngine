#pragma once

#include "FormationBehaviour.h"

#include <glm/glm.hpp>

#include <vector>

class Coordinator
{
public:
	Coordinator(const std::vector<FormationBehaviour*>& formationEntities, std::vector<glm::vec3>& offsets, const glm::vec3& center);
	Coordinator(const glm::vec3& center = glm::vec3(0.0f));
	~Coordinator(); 

	void Update(float deltaTime);

	void SetCentralPosition(const glm::vec3& pos) { centralPosition = pos; }
	void AddFormationEntity(FormationBehaviour* behaviour) { formationEntities.push_back(behaviour); }

	void AddPositionOffset(const glm::vec3& offset) { offsets.push_back(offset); }
	void SetPositionOffsets(const std::vector<glm::vec3>& offsets) { this->offsets = offsets; }

private:
	std::vector<FormationBehaviour*> formationEntities;
	std::vector<glm::vec3> offsets;
	glm::vec3 centralPosition;
};