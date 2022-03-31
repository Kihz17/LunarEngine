#pragma once

#include "IMesh.h"
#include "Component.h"
#include "ITexture.h"
#include "ReflectRefract.h"
#include "Shader.h"

#include <glm/glm.hpp>

enum class FaceCullType
{
	None,
	Back, 
	Front
};

struct RenderComponent : public Component
{
	struct RenderInfo
	{
		IMesh* mesh = nullptr;

		bool isColorOverride = false;
		glm::vec3 colorOverride = glm::vec3(0.0f);
		std::vector<std::pair<ITexture*, float>> albedoTextures;

		ITexture* normalTexture = nullptr;
		ITexture* ormTexture = nullptr;

		glm::vec2 uvOffset = glm::vec2(1.0f, 1.0f);

		float roughness = 0.01f;
		float metalness = 0.02f;
		float ao = 1.0f;

		bool isWireframe = false;
		bool isIgnoreLighting = false;

		float alphaTransparency = 1.0f;

		bool castShadows = true;
		bool castShadowsOn = true;
		float surfaceShadowSoftness = 1.0f;
		float castingShadownSoftness = 0.7f;

		ReflectRefractType reflectRefractType = ReflectRefractType::None;
		ReflectRefractMapType reflectRefractMapType = ReflectRefractMapType::Environment;
		CubeMap* reflectRefractCustomMap = nullptr;
		float reflectRefractStrength = 0.0f;
		float refractRatio = 0.0f;
		ReflectRefractMapPriorityType reflectRefractMapPriority = ReflectRefractMapPriorityType::High;

		FaceCullType faceCullType = FaceCullType::Back;
	};

	RenderComponent()
		: mesh(nullptr),
		isColorOverride(false),
		colorOverride(glm::vec3(0.0f)),
		normalTexture(nullptr),
		ormTexture(nullptr),
		roughness(0.01f),
		metalness(0.02f),
		ao(1.0f),
		uvOffset(glm::vec2(1.0f, 1.0f)),
		isWireframe(false),
		isIgnoreLighting(false),
		alphaTransparency(1.0f),
		hasPrevProjViewModel(false),
		projViewModel(glm::mat4(1.0f)),
		castShadows(true),
		castShadowsOn(true),
		surfaceShadowSoftness(1.0f),
		castingShadownSoftness(0.75f),
		reflectRefractMapPriority(ReflectRefractMapPriorityType::High),
		faceCullType(FaceCullType::Back)
	{}

	RenderComponent(const RenderInfo& renderInfo)
		: mesh(renderInfo.mesh),
		isColorOverride(renderInfo.isColorOverride),
		colorOverride(renderInfo.colorOverride),
		albedoTextures(renderInfo.albedoTextures),
		normalTexture(renderInfo.normalTexture),
		ormTexture(renderInfo.ormTexture),
		roughness(renderInfo.roughness),
		metalness(renderInfo.metalness),
		ao(renderInfo.ao),
		uvOffset(renderInfo.uvOffset),
		isWireframe(renderInfo.isWireframe),
		isIgnoreLighting(renderInfo.isIgnoreLighting),
		alphaTransparency(renderInfo.alphaTransparency),
		hasPrevProjViewModel(false),
		projViewModel(glm::mat4(1.0f)),
		castShadows(renderInfo.castShadows),
		castShadowsOn(renderInfo.castShadowsOn),
		surfaceShadowSoftness(renderInfo.surfaceShadowSoftness),
		castingShadownSoftness(renderInfo.castingShadownSoftness),
		reflectRefractData(renderInfo.reflectRefractType, renderInfo.reflectRefractMapType, renderInfo.reflectRefractCustomMap, renderInfo.reflectRefractStrength, renderInfo.refractRatio),
		reflectRefractMapPriority(renderInfo.reflectRefractMapPriority),
		faceCullType(renderInfo.faceCullType)
	{}

	bool HasMaterialTextures() const
	{
		return ormTexture;
	}

	void Draw(const Shader* shader, const glm::mat4& transform)
	{
		if (isWireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		mesh->GetVertexArray()->Bind();
		for (Submesh& submesh : mesh->GetSubmeshes())
		{
			shader->SetMat4("uMatModel", transform);
			glDrawElementsBaseVertex(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(int) * submesh.indexStart), submesh.vertexStart);
		}
	
		mesh->GetVertexArray()->Unbind();
	}

	IMesh* mesh;

	// Diffuse color
	bool isColorOverride;
	glm::vec3 colorOverride;
	std::vector<std::pair<ITexture*, float>> albedoTextures;

	// Materials Textures
	ITexture* normalTexture;
	ITexture* ormTexture;

	glm::vec2 uvOffset;

	// Material values (should only be used if material textures are not present)
	float roughness;
	float metalness;
	float ao;

	bool isWireframe;
	bool isIgnoreLighting;

	float alphaTransparency;

	bool hasPrevProjViewModel;
	glm::mat4 projViewModel;

	bool castShadows;
	bool castShadowsOn;
	float surfaceShadowSoftness;
	float castingShadownSoftness;

	ReflectRefractData reflectRefractData;
	ReflectRefractMapPriorityType reflectRefractMapPriority;

	FaceCullType faceCullType;
};