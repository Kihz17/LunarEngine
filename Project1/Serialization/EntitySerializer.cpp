#include "EntitySerializer.h"
#include "EntityComponentSerializer.h"

EntitySerializer::EntitySerializer(Entity* entity, EntityManager& entityManager)
	: entity(entity),
	entityManager(entityManager)
{

}

void EntitySerializer::Serialize(YAML::Emitter& emitter)
{
	emitter << YAML::BeginMap;

	emitter << YAML::Key << "Name" << YAML::Value << entity->name;
	emitter << YAML::Key << "ID" << YAML::Value << entity->id;

	emitter << YAML::Key << "Components" << YAML::Value << YAML::BeginSeq;
	for (Component* c : entity->components) EntityComponentSerializer(c, entity).Serialize(emitter);
	emitter << YAML::EndSeq;

	emitter << YAML::EndMap;
}

void EntitySerializer::Deserialize(const YAML::Node& node)
{
	std::string name = node["Name"].as<std::string>();
	int id = node["ID"].as<int>();

	Entity* e = entityManager.CreateEntity(name);
	e->id = id;

	const YAML::Node& components = node["Components"];
	if (components)
	{
		YAML::const_iterator it;
		for (it = components.begin(); it != components.end(); it++)
		{
			YAML::Node childNode = (*it);
			EntityComponentSerializer(nullptr, e).Deserialize(childNode);
		}
	}
}