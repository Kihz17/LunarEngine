#pragma once

#include "ISerializer.h"
#include "Components.h"
#include "Entity.h"

class EntityComponentSerializer : public ISerializer
{
public:
	EntityComponentSerializer(Component* c, Entity* e);

	virtual void Serialize(YAML::Emitter& emitter) override;
	virtual void Deserialize(YAML::Node& node) override;

private:
	void SavePositionComponent(YAML::Emitter& emitter, PositionComponent* posComp);
	void SaveRotationComponent(YAML::Emitter& emitter, RotationComponent* rotComp);
	void SaveScaleComponent(YAML::Emitter& emitter, ScaleComponent* scaleComp);
	void SaveRenderComponent(YAML::Emitter& emitter, RenderComponent* renderComp);
	void SaveRigidComponent(YAML::Emitter& emitter, RigidBodyComponent* rigidComp);

	Component* component;
	Entity* entity;
};