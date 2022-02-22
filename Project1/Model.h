#pragma once

#include "IMesh.h"
#include "AABB.h"
#include "ITexture.h"
#include "ReflectRefract.h"

struct Submesh
{
	IMesh* mesh = nullptr;
	glm::mat4 localTransform = glm::mat4(1.0f);

	bool isColorOverride = false;
	glm::vec3 colorOverride = glm::vec3(1.0f);
	std::vector<std::pair<ITexture*, float>> albedoTextures;

	ITexture* normalTexture = nullptr;
	ITexture* roughnessTexture = nullptr;
	ITexture* metalTexture = nullptr;
	ITexture* aoTexture = nullptr;

	float roughness = 0.01f;
	float metalness = 0.02f;
	float ao = 1.0f;

	bool isWireframe = false;
	bool isIgnoreLighting = false;

	float alphaTransparency = 1.0f;

	bool castShadows = true;
	bool castShadowsOn = true;
	float shadowSoftness = 1.0f;

	ReflectRefractType reflectRefractType = ReflectRefractType::None;
	ReflectRefractMapType reflectRefractMapType = ReflectRefractMapType::Environment;
	CubeMap* reflectRefractCustomMap = nullptr;
	float reflectRefractStrength = 0.0f;
	float refractRatio = 0.0f;
	ReflectRefractMapPriorityType reflectRefractMapPriority = ReflectRefractMapPriorityType::High;

	bool HasMaterialTextures() const
	{
		return roughnessTexture && metalTexture && aoTexture;
	}
};

class Model
{
public:
	Model(const std::string& filePath);
	virtual ~Model();

	std::vector<Submesh>& GetSubmeshes() { return submeshes; }
	const AABB* GetBoundingBox() const { return boundingBox; }

private:
	void ParseNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform);

	std::vector<Submesh> submeshes;

	AABB* boundingBox;

	std::string filePath;
};