#pragma once

#include "IMesh.h"
#include "AABB.h"
#include "ITexture.h"
#include "ReflectRefract.h"
#include "Bone.h"
#include "IVertex.h"
#include "BoneInfo.h"

#include <assimp/scene.h>

#include <map>
#include <unordered_map>

class AnimatedMesh : public IMesh
{
public:
	AnimatedMesh(const std::string& filePath);
	virtual ~AnimatedMesh();

	virtual std::vector<Submesh>& GetSubmeshes() override { return submeshes; }

	virtual VertexArrayObject* GetVertexArray() override { return this->vertexArray; }
	virtual VertexBuffer* GetVertexBuffer() override { return this->vertexBuffer; }
	virtual IndexBuffer* GetIndexBuffer() override { return this->indexBuffer; }

	virtual const BufferLayout& GetVertexBufferLayout() const override { return this->vertexBuffer->GetLayout(); }

	virtual const std::vector<IVertex*>& GetVertices() const override { return this->vertices; }
	virtual const std::vector<Face>& GetFaces() const override { return this->indices; }

	virtual const AABB* GetBoundingBox() const override { return this->boundingBox; }

	const BoneInfo* GetBoneInfo(const std::string& boneName) const;
	unsigned int GetBoneCount() const { return boneCount; }
	const Bone& GetRootBone() const { return rootBone; }
	const glm::mat4& GetInverseTransform() const { return inverseTransform; }

private:
	friend class Animation;

	void ParseMesh(unsigned int meshIndex, const aiMesh* assimpMesh);
	void ParseBones(unsigned int meshIndex, const aiMesh* assimpMesh);
	void ParseMaterials(const aiScene* assimpScene);
	void ParseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f));

	void CreateBoneHierarchy(aiNode* node, Bone& parentBone, const glm::mat4& parentTransform);

	VertexArrayObject* vertexArray;
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

	std::vector<IVertex*> vertices;
	std::vector<Face> indices;

	std::vector<Submesh> submeshes;
	AABB* boundingBox;
	std::string filePath;

	std::unordered_map<std::string, BoneInfo> boneMap;
	unsigned int boneCount;
	Bone rootBone;
	glm::mat4 inverseTransform;
};