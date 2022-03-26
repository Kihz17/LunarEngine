#pragma once

#include "ISerializer.h"
#include "Entity.h"
#include "EntityManager.h"

class EntitySerializer : public ISerializer
{
public:
	EntitySerializer(Entity* entity, EntityManager& entityManager);

	virtual void Serialize(YAML::Emitter& emitter) override;
	virtual void Deserialize(const YAML::Node& node) override;

private:
	Entity* entity;
	EntityManager& entityManager;
};