#pragma once

#include "ApplicationLayer.h"
#include "EntityManager.h"
#include "Components.h"

#include <IPhysicsWorld.h>

class EditorLayer : public ApplicationLayer
{
public:
	EditorLayer(EntityManager& entityManager, Physics::IPhysicsWorld<Entity>* physWorld);

	virtual void OnUpdate(float deltaTime) override;

private:
	void ShowEntity(Entity* entity);
	void ShowComponent(Component* c);

	EntityManager& entityManager;
	Physics::IPhysicsWorld<Entity>* physWorld;

	int currentComponent;
	int currentTextureFilter;
	int currentTextureWrap;
	int currentMatTextureFilter;
	int currentMatTextureWrap;
};