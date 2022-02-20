#include "Coordinator.h"

Coordinator::Coordinator(const std::vector<FormationBehaviour*>& formationEntities, std::vector<glm::vec3>& offsets, const glm::vec3& center)
	: formationEntities(formationEntities),
	offsets(offsets),
	centralPosition(center)
{

}

Coordinator::Coordinator(const glm::vec3& center)
	: centralPosition(center)
{

}

Coordinator::~Coordinator()
{

}

void Coordinator::Update(float deltaTime)
{
	std::vector<FormationBehaviour*> formationCopy = formationEntities;

	for (int i = 0; i < offsets.size(); i++)
	{
		if (formationCopy.empty()) break;

		glm::vec3 position = centralPosition + offsets[i];

		FormationBehaviour* closestEntity = nullptr;
		float closestDistance = std::numeric_limits<float>::max();
		int closestIndex = 0;
		for (int j = 0; j < formationCopy.size(); j++)
		{
			FormationBehaviour* formationEntity = formationCopy[j];
			float distance = glm::length(position - formationEntity->GetRigidBody()->GetPosition());
			if (distance < closestDistance)
			{
				closestEntity = formationEntity;
				closestDistance = distance;
				closestIndex = j;
			}
		}

		if (closestEntity)
		{
			closestEntity->targetPos = position;
			formationCopy.erase(formationCopy.begin() + closestIndex);
		}	
	}
}