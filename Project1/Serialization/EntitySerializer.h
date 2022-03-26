#pragma once

#include "ISerializer.h"
#include "Entity.h"

class EntitySerializer : public ISerializer
{
public:
	EntitySerializer(Entity* entity);

	virtual void Serialize(YAML::Emitter& emitter) override;
	virtual void Deserialize(YAML::Node& node) override;

private:
	Entity* entity;
};