#include "AnimatedMesh.h"
#include "Vertex.h"
#include "AnimatedVertex.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>

struct AssimpLogger : public Assimp::LogStream
{
	static void Initialize()
	{
		if (Assimp::DefaultLogger::isNullLogger())
		{
			Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
			Assimp::DefaultLogger::get()->attachStream(new AssimpLogger, Assimp::Logger::Err | Assimp::Logger::Warn);
		}
	}

	virtual void write(const char* message) override
	{
		std::cout << "[ASSIMP ERROR]: " << message;
	}
};

AnimatedMesh::AnimatedMesh(const std::string& filePath)
	: vertexArray(nullptr),
	vertexBuffer(nullptr),
	indexBuffer(nullptr),
	filePath(filePath), 
	boundingBox(nullptr),
	boneCount(0)
{
	//AssimpLogger::Initialize();

	std::cout << "Loading animated mesh " << filePath << "..." << std::endl;;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filePath, MeshUtils::ASSIMP_FLAGS);
	if (!scene || !scene->HasMeshes())
	{
		std::cout << "Failed to load animated mesh file: " << filePath << std::endl;
		return;
	}

	this->submeshes.resize(scene->mNumMeshes);

	// Track the number of vertices and indices (this will be important later when drawing the model)
	unsigned int vertexCount = 0;
	unsigned int indexCount = 0;
	for (unsigned int i = 0; i < submeshes.size(); i++)
	{
		submeshes[i].materialIndex = scene->mMeshes[i]->mMaterialIndex;

		submeshes[i].vertexStart = vertexCount;
		submeshes[i].indexStart = indexCount;

		submeshes[i].indexCount = scene->mMeshes[i]->mNumFaces * 3;

		vertexCount += scene->mMeshes[i]->mNumVertices;
		indexCount += submeshes[i].indexCount;
	}

	vertices.reserve(vertexCount);
	indices.reserve(indexCount);

	for (unsigned int i = 0; i < submeshes.size(); i++)
	{
		const aiMesh* assimpMesh = scene->mMeshes[i];
		ParseMesh(i, assimpMesh);
	}

	ParseMaterials(scene);

	inverseTransform = MeshUtils::ConvertToGLMMat4(scene->mRootNode->mTransformation.Inverse());
	ParseNodes(scene->mRootNode);
	CreateBoneHierarchy(scene->mRootNode, rootBone, glm::mat4(1.0f));

	// Configure parent's bounding box based off of the submeshes we just added
	glm::vec3 parentMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 parentMax = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (Submesh& submesh : this->submeshes)
	{
		const glm::mat4& localTransform = submesh.localTransform;
		glm::vec4 min = localTransform * glm::vec4(submesh.minVertex, 1.0f);
		glm::vec4 max = localTransform * glm::vec4(submesh.maxVertex, 1.0f);

		parentMin.x = glm::min(parentMin.x, min.x);
		parentMin.y = glm::min(parentMin.y, min.y);
		parentMin.z = glm::min(parentMin.z, min.z);
		parentMax.x = glm::max(parentMax.x, max.x);
		parentMax.y = glm::max(parentMax.y, max.y);
		parentMax.z = glm::max(parentMax.z, max.z);
	}

	this->boundingBox = new AABB(parentMin, parentMax, true);

	// Define vertex layout
	BufferLayout bufferLayout;
	if (MAX_BONE_INFLUENCE <= 4)
	{
		bufferLayout = {
			{ ShaderDataType::Float3, "vPosition" },
			{ ShaderDataType::Float3, "vNormal" },
			{ ShaderDataType::Float2, "vTextureCoordinates" },
			{ ShaderDataType::Float4, "vBoneIDs" },
			{ ShaderDataType::Float4, "vBoneWeights" }
		};
	}
	else
	{
		std::cout << "Max bone influence was greater than 4!\n";
		return;
	}

	this->vertexArray = new VertexArrayObject();

	int s = AnimatedVertex::Size();
	uint32_t vertexBufferSize = (uint32_t)(this->vertices.size() * AnimatedVertex::Size());
	float* vertexBuffer = MeshUtils::ConvertAnimatedVerticesToArray(this->vertices);

	this->vertexBuffer = new VertexBuffer(vertexBuffer, vertexBufferSize);
	this->vertexBuffer->SetLayout(bufferLayout);

	uint32_t indexBufferSize = (uint32_t)(this->indices.size() * sizeof(Face));
	uint32_t* indexBuffer = MeshUtils::ConvertIndicesToArray(this->indices);

	this->vertexArray->Bind();
	this->indexBuffer = new IndexBuffer(indexBuffer, indexBufferSize);

	this->vertexArray->AddVertexBuffer(this->vertexBuffer);
	this->vertexArray->SetIndexBuffer(this->indexBuffer);

	delete[] vertexBuffer;
	delete[] indexBuffer;
}

AnimatedMesh::~AnimatedMesh()
{
	delete vertexArray;
	delete vertexBuffer;
	delete indexBuffer;

	for (IVertex* v : vertices) delete v;

	delete boundingBox;
}

void AnimatedMesh::ParseMesh(unsigned int meshIndex, const aiMesh* assimpMesh)
{
	if (!assimpMesh->HasPositions())
	{
		std::cout << filePath << " does not have position!" << std::endl;
		return;
	}

	if (!assimpMesh->HasNormals())
	{
		std::cout << filePath << " does not have normals!" << std::endl;
		return;
	}

	if(!assimpMesh->HasTextureCoords(0))
	{
		std::cout << filePath << " does not have texture coords!" << std::endl;
	}

	int32_t defaultBoneIDs[MAX_BONE_INFLUENCE];
	for (unsigned int j = 0; j < MAX_BONE_INFLUENCE; j++) defaultBoneIDs[j] = -1;

	float defaultBoneWeights[MAX_BONE_INFLUENCE];
	for (unsigned int j = 0; j < MAX_BONE_INFLUENCE; j++) defaultBoneWeights[j] = 0.0f;

	glm::vec3 minVertex = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 maxVertex = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
	{
		glm::vec3 pos = glm::vec3(assimpMesh->mVertices[i].x, assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z);
		glm::vec3 normal = glm::vec3(assimpMesh->mNormals[i].x, assimpMesh->mNormals[i].y, assimpMesh->mNormals[i].z);
		glm::vec3 texCoords(0.0f);
		if (assimpMesh->HasTextureCoords(0))
		{
			texCoords.x = assimpMesh->mTextureCoords[0][i].x;
			texCoords.y = assimpMesh->mTextureCoords[0][i].y;
		}

		minVertex.x = glm::min(minVertex.x, pos.x);
		minVertex.y = glm::min(minVertex.y, pos.y);
		minVertex.z = glm::min(minVertex.z, pos.z);

		maxVertex.x = glm::max(maxVertex.x, pos.x);
		maxVertex.y = glm::max(maxVertex.y, pos.y);
		maxVertex.z = glm::max(maxVertex.z, pos.z);

		vertices.push_back(new AnimatedVertex(pos, normal, texCoords, defaultBoneIDs, defaultBoneWeights));
	}

	submeshes[meshIndex].minVertex = minVertex;
	submeshes[meshIndex].maxVertex = maxVertex;

	ParseBones(meshIndex, assimpMesh);

	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++)
	{
		const aiFace& assimpFace = assimpMesh->mFaces[i];
		if (assimpFace.mNumIndices != 3)
		{
			std::cout << "Face must be a triangle!" << std::endl;
			return;
		}

		Face face;
		face.v1 = assimpFace.mIndices[0];
		face.v2 = assimpFace.mIndices[1];
		face.v3 = assimpFace.mIndices[2];
		indices.push_back(face);
	}
}

void AnimatedMesh::ParseBones(unsigned int meshIndex, const aiMesh* assimpMesh)
{
	for (unsigned int i = 0; i < assimpMesh->mNumBones; i++)
	{
		const aiBone* assimpBone = assimpMesh->mBones[i];

		int32_t boneIndex = 0;
		std::string boneName = assimpBone->mName.C_Str();

		if (boneMap.find(boneName) == boneMap.end()) // This bone hasn't been mapped out yet
		{
			boneIndex = boneCount;
			boneCount++;

			BoneInfo info;
			info.boneID = boneIndex;
			info.bindTransform = MeshUtils::ConvertToGLMMat4(assimpBone->mOffsetMatrix);
			boneMap.insert({ boneName, info });
		}
		else // Bone has laready been mapped
		{
			boneIndex = boneMap[boneName].boneID;
		}

		for (unsigned int j = 0; j < assimpBone->mNumWeights; j++)
		{
			unsigned int vertexID = submeshes[meshIndex].vertexStart + assimpBone->mWeights[j].mVertexId;
			float weight = assimpBone->mWeights[j].mWeight;
			dynamic_cast<AnimatedVertex*>(vertices[vertexID])->AddBoneData(boneIndex, weight);
		}
	}
}

void AnimatedMesh::ParseMaterials(const aiScene* assimpScene)
{
	// TODO
}

void AnimatedMesh::ParseNodes(aiNode* node, const glm::mat4& parentTransform)
{
	glm::mat4 transform = parentTransform * MeshUtils::ConvertToGLMMat4(node->mTransformation);
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		unsigned int meshIndex = node->mMeshes[i];
		Submesh& submesh = this->submeshes[meshIndex];
		submesh.nodeName = node->mName.C_Str();
		submesh.localTransform = transform;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ParseNodes(node->mChildren[i], transform);
	}
}

const BoneInfo* AnimatedMesh::GetBoneInfo(const std::string& boneName) const
{
	std::unordered_map<std::string, BoneInfo>::const_iterator it = boneMap.find(boneName);
	if (it == boneMap.end()) return nullptr;
	return &it->second;
}

void AnimatedMesh::CreateBoneHierarchy(aiNode* node, Bone& parentBone, const glm::mat4& parentTransform)
{
	std::string nodeName = node->mName.C_Str();
	std::unordered_map<std::string, BoneInfo>::iterator it = boneMap.find(nodeName);

	if (it != boneMap.end()) // This node is a bone
	{
		if (parentBone.ID == -1) // We haven't found root bone yet
		{
			parentBone.ID = it->second.boneID;
			parentBone.name = it->first;
			parentBone.offsetTransform = it->second.bindTransform;
			parentBone.bindTransform = parentTransform * it->second.bindTransform;

			// We didn't add anything here, so keep the parent bone the same
			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				CreateBoneHierarchy(node->mChildren[i], parentBone, parentTransform);
			}
		}
		else
		{
			// Check if the child vector already has this bone
			// As annoying as it is, we need to check this because there can be nodes with the same name that aren't neccessarily bones
			// Example: https://gyazo.com/36b002d46d2f6610338707261e2c0fad
			bool alreadyHas = false;
			for (Bone& childBone : parentBone.children)
			{
				if (childBone.name == it->first && childBone.ID == it->second.boneID)
				{
					alreadyHas = true;
					break;
				}
			}

			if (!alreadyHas)
			{
				Bone newBone;
				newBone.ID = it->second.boneID;
				newBone.name = it->first;
				newBone.offsetTransform = it->second.bindTransform;
				newBone.bindTransform = parentTransform * it->second.bindTransform;

				parentBone.children.push_back(newBone);
				Bone& newParentBone = parentBone.children[parentBone.children.size() - 1];

				// Recursively add children to the new bone
				for (unsigned int i = 0; i < node->mNumChildren; i++)
				{
					CreateBoneHierarchy(node->mChildren[i], newParentBone, newParentBone.bindTransform);
				}
			}
			
		}
	}
	else // No bone, keep continuing with the same parent bone
	{
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			CreateBoneHierarchy(node->mChildren[i], parentBone, parentTransform);
		}
	}
}