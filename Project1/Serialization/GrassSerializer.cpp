#include "GrassSerializer.h"
#include "TextureManager.h"

GrassSerializer::GrassSerializer(GrassCluster& cluster)
	: cluster(cluster)
{

}

void GrassSerializer::Serialize(YAML::Emitter& emitter)
{
	emitter << YAML::BeginMap;

	emitter << YAML::Key << "AlbedoTexture" << YAML::Value << cluster.albedoTexture->GetPath();
	if (cluster.discardTexture) emitter << YAML::Key << "DiscardTexture" << YAML::Value << cluster.discardTexture->GetPath();
	if (cluster.normalTexture) emitter << YAML::Key << "NormalTexture" << YAML::Value << cluster.normalTexture->GetPath();

	emitter << YAML::Key << "Roughness" << YAML::Value << cluster.roughness;
	emitter << YAML::Key << "Metalness" << YAML::Value << cluster.metalness;
	emitter << YAML::Key << "AmbientOcclusion" << YAML::Value << cluster.ao;
	emitter << YAML::Key << "WindDirection" << YAML::Value << cluster.windDirection;
	emitter << YAML::Key << "OscillationStrength" << YAML::Value << cluster.oscillationStrength;
	emitter << YAML::Key << "WindForce" << YAML::Value << cluster.windForceMult;
	emitter << YAML::Key << "Stiffness" << YAML::Value << cluster.stiffness;
	emitter << YAML::Key << "Dimensions" << YAML::Value << cluster.dimensions;

	emitter << YAML::Key << "Data" << YAML::Value << cluster.grassData;

	emitter << YAML::EndMap;
}

void GrassSerializer::Deserialize(const YAML::Node& node)
{
	if (!node["AlbedoTexture"]) return;

	cluster.albedoTexture = TextureManager::CreateTexture2D(node["AlbedoTexture"].as<std::string>(), TextureFilterType::Linear, TextureWrapType::ClampToEdge);

	if (node["DiscardTexture"])
	{
		cluster.discardTexture = TextureManager::CreateTexture2D(node["DiscardTexture"].as<std::string>(), TextureFilterType::Linear, TextureWrapType::ClampToEdge);
	}

	if (node["NormalTexture"])
	{
		cluster.normalTexture = TextureManager::CreateTexture2D(node["NormalTexture"].as<std::string>(), TextureFilterType::Linear, TextureWrapType::ClampToEdge);
	}

	cluster.roughness = node["Roughness"].as<float>();
	cluster.metalness = node["Metalness"].as<float>();
	cluster.ao = node["AmbientOcclusion"].as<float>();
	cluster.windDirection = node["WindDirection"].as<glm::vec2>();
	cluster.oscillationStrength = node["OscillationStrength"].as<float>();
	cluster.windForceMult = node["WindForce"].as<float>();
	cluster.stiffness = node["Stiffness"].as<float>();
	cluster.dimensions = node["Dimensions"].as<glm::vec2>();

	cluster.grassData = node["Data"].as<std::vector<glm::vec4>>();

	BufferLayout bufferLayout = {
		{ ShaderDataType::Float4, "vWorldPosition" }
	};

	cluster.VAO = new VertexArrayObject();
	cluster.VBO = new VertexBuffer(cluster.grassData.size() * sizeof(glm::vec4));
	cluster.VBO->SetData(cluster.grassData.data(), cluster.grassData.size() * sizeof(glm::vec4));
	cluster.VBO->SetLayout(bufferLayout);
	cluster.VAO->AddVertexBuffer(cluster.VBO);
}