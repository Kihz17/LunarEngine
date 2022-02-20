#pragma once

#include "Mesh.h"
#include "ApplicationLayer.h"
#include "EntityManager.h"
#include "Coordinator.h"
#include "FlockingBehaviour.h"
#include "Path.h"

#include <IPhysicsFactory.h>
#include <glm/glm.hpp>

struct FormationLayerInfo
{
	Mesh* mesh;
	Mesh* pathMeshes;
	EntityManager* entityManager;
	Physics::IPhysicsFactory<Entity>* physicsFactory;
	Physics::IPhysicsWorld<Entity>* physicsWorld;
};

enum class FlockingWeightType
{
	Cohesion,
	Separation,
	Alignment
};

class FormationLayer : public ApplicationLayer
{
public:
	FormationLayer(const FormationLayerInfo& info);
	~FormationLayer();

	virtual void OnAttach() override;
	virtual void OnUpdate(float deltaTime) override;

private:
	void SetFlocking(bool b);
	Entity* SpawnTestEntity(const glm::vec3& position);
	void ChangeFlockingWeight(float modification, FlockingWeightType type);
	void UpdateFlockingDirection(const glm::vec3& pos);

	Mesh* entityMesh;
	Mesh* pathMesh;
	EntityManager* entityManager;
	Physics::IPhysicsFactory<Entity>* physicsFactory;
	Physics::IPhysicsWorld<Entity>* physicsWorld;

	Coordinator coordinator;
	FlockingGroup* flockingGroup;

	Path path;
	bool followPath;
	glm::vec3 pathPos;
	std::vector<Entity*> pathNodeEntities;
};