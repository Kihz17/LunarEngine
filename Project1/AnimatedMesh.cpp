#include "AnimatedMesh.h"

#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>

#include <iostream>

struct AnimatedAssimpLogger : public Assimp::LogStream
{
	static void Initialize()
	{
		if (Assimp::DefaultLogger::isNullLogger())
		{
			Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
			Assimp::DefaultLogger::get()->attachStream(new AnimatedAssimpLogger, Assimp::Logger::Err | Assimp::Logger::Warn);
		}
	}

	virtual void write(const char* message) override
	{
		std::cout << "[ASSIMP ERROR]: " << message;
	}
};

AnimatedMesh::AnimatedMesh(const std::string& path)
	: filePath(path),
	boundingBox(nullptr),
	boneCount(0)
{
	//AnimatedAssimpLogger::Initialize();

	std::cout << "Loading animated mesh " << path << "...\n";

	importer = CreateScope<Assimp::Importer>();

	const aiScene* scene = importer->ReadFile(path, MeshUtils::ASSIMP_FLAGS);
	if (!scene || !scene->HasMeshes())
	{
		std::cout << "Failed to load mesh file: " << filePath << std::endl;
		return;
	}

	this->assimpScene = scene;
	this->submeshes.reserve(scene->mNumMeshes);

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* assimpMesh = scene->mMeshes[i];

		// Create a submesh
		Submesh submesh;
		submesh.baseVertex = vertexCount; // Assign at what index this submeshes vertices start
		submesh.baseIndex = indexCount; // Assign at what index this submeshes indices start
		submesh.materialIndex = assimpMesh->mMaterialIndex; // Assign at what index this submeshes materials start
		submesh.vertexCount = assimpMesh->mNumVertices;
		submesh.indexCount = assimpMesh->mNumFaces * 3;
		submesh.meshName = assimpMesh->mName.C_Str();

		vertexCount += assimpMesh->mNumVertices;
		indexCount += submesh.indexCount;

		if (!assimpMesh->HasPositions())
		{
			std::cout << path << " does not have position!" << std::endl;
			return;
		}

		if (!assimpMesh->HasNormals())
		{
			std::cout << path << " does not have normals!" << std::endl;
			return;
		}

		// Define min/max for this meshes bounding box
		glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		for (unsigned int j = 0; j < assimpMesh->mNumVertices; j++)
		{
			glm::vec3 position = glm::vec3(assimpMesh->mVertices[j].x, assimpMesh->mVertices[j].y, assimpMesh->mVertices[j].z);
			glm::vec3 normal = glm::vec3(assimpMesh->mNormals[j].x, assimpMesh->mNormals[j].y, assimpMesh->mNormals[j].z);

			// Find the lowest and highest points of the bounding box
			min.x = glm::min(position.x, min.x);
			min.y = glm::min(position.y, min.y);
			min.z = glm::min(position.z, min.z);
			max.x = glm::max(position.x, max.x);
			max.y = glm::max(position.y, max.y);
			max.z = glm::max(position.z, max.z);

			glm::vec2 textureCoords(0.0f);
			if (assimpMesh->HasTextureCoords(0))
			{
				textureCoords.x = assimpMesh->mTextureCoords[0][j].x;
				textureCoords.y = assimpMesh->mTextureCoords[0][j].y;
			}

			int defaultBoneIDs[MAX_BONE_INFLUENCE];
			for (unsigned int j = 0; j < MAX_BONE_INFLUENCE; j++) defaultBoneIDs[j] = -1;

			float defaultBoneWeights[MAX_BONE_INFLUENCE];
			for (unsigned int j = 0; j < MAX_BONE_INFLUENCE; j++) defaultBoneWeights[j] = 0.0f;

			this->vertices.push_back(new AnimatedVertex(position, normal, textureCoords, defaultBoneIDs, defaultBoneWeights));
		}

		submesh.boundingBox->Resize(min, max); // Resize the bounding box to the proper size

		// Assign faces/indices
		for (unsigned int j = 0; j < assimpMesh->mNumFaces; j++)
		{
			if (assimpMesh->mFaces[j].mNumIndices != 3)
			{
				std::cout << assimpMesh->mName.C_Str() << " " << assimpMesh->mFaces[j].mNumIndices << std::endl;
				std::cout << "Face must be a triangle! " << j << std::endl;
				return;
			}

			Face face;
			face.v1 = assimpMesh->mFaces[j].mIndices[0];
			face.v2 = assimpMesh->mFaces[j].mIndices[1];
			face.v3 = assimpMesh->mFaces[j].mIndices[2];
			this->faces.push_back(face);
		}

		// Assign bones
		for (unsigned int j = 0; j < assimpMesh->mNumBones; j++)
		{
			aiBone* assimpBone = assimpMesh->mBones[j];
			std::string boneName = assimpBone->mName.C_Str();

			int boneID = -1;
			if (boneInfo.find(boneName) != boneInfo.end()) // We already have this bone cached
			{
				boneID = boneInfo[boneName].id;
			}
			else // We haven't cached this bone data yet, store it
			{
				// Create new bone info
				BoneInfo info;
				info.id = boneCount;
				info.offset = MeshUtils::ConvertToGLMMat4(assimpBone->mOffsetMatrix);
				boneInfo.insert({boneName, info});

				boneID = boneCount; // Assign bone ID to current count
				boneCount++; // Increment bone count
			}

			if (boneID == -1)
			{
				std::cout << "WARNING: Bone ID for mesh " << path << " is -1.\n";
			}

			// Map bone to specific vertex with weight
			for (unsigned int k = 0; k < assimpBone->mNumWeights; k++)
			{
				int vertexID = assimpBone->mWeights[k].mVertexId;
				float weight = assimpBone->mWeights[k].mWeight;

				if (vertexID >= vertices.size())
				{
					std::cout << "WARNING: Bone vertex ID was greater than number of vertices for " << path << ".\n";
				}

				for (unsigned int l = 0; l < MAX_BONE_INFLUENCE; l++)
				{
					AnimatedVertex* v = dynamic_cast<AnimatedVertex*>(vertices[vertexID]);
					if (v->Data()[BONE_ID_START_INDEX + l] >= 0) continue; // This index has already been assigned data
					
					v->Data()[BONE_ID_START_INDEX + l] = boneID;
					v->Data()[BONE_WEIGHT_START_INDEX + l] = weight;
					break;
				}
			}
		}

		this->submeshes.push_back(submesh);
	}

	LoadNodes(scene->mRootNode); // Load all the submeshes

	// Configure parent's bounding box based off of the submeshes we just added
	glm::vec3 parentMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 parentMax = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (Submesh& submesh : this->submeshes)
	{
		AABB* submeshAABB = submesh.boundingBox;
		glm::vec3 submeshMin = submeshAABB->GetMin();
		glm::vec3 submeshMax = submeshAABB->GetMax();

		glm::vec3 min = glm::vec3(submesh.transform * glm::vec4(submeshMin, 1.0f));
		glm::vec3 max = glm::vec3(submesh.transform * glm::vec4(submeshMax, 1.0f));

		parentMin.x = glm::min(parentMin.x, min.x);
		parentMin.y = glm::min(parentMin.y, min.y);
		parentMin.z = glm::min(parentMin.z, min.z);
		parentMax.x = glm::max(parentMax.x, max.x);
		parentMax.y = glm::max(parentMax.y, max.y);
		parentMax.z = glm::max(parentMax.z, max.z);
	}
	this->boundingBox = new AABB(parentMin, parentMax);

	if (scene->HasMaterials())
	{
		SetupMaterials();
	}

	BufferLayout bufferLayout;
	if (MAX_BONE_INFLUENCE <= 4)
	{
		bufferLayout  = {
			{ ShaderDataType::Float3, "vPosition" },
			{ ShaderDataType::Float3, "vNormal" },
			{ ShaderDataType::Float2, "vTextureCoordinates" },
			{ ShaderDataType::Int4, "vBoneIDs" },
			{ ShaderDataType::Float4, "vBoneWeights" }
		};
	}
	else if(MAX_BONE_INFLUENCE >= 5)
	{
		bufferLayout = {
			{ ShaderDataType::Float3, "vPosition" },
			{ ShaderDataType::Float3, "vNormal" },
			{ ShaderDataType::Float2, "vTextureCoordinates" },
			{ ShaderDataType::Int4, "vBoneIDs1" },
			{ ShaderDataType::Int4, "vBoneIDs2" },
			{ ShaderDataType::Float4, "vBoneWeights1" },
			{ ShaderDataType::Float4, "vBoneWeights2" }
		};
	}

	this->vertexArray = new VertexArrayObject();

	uint32_t vertexBufferSize = (uint32_t)(this->vertices.size() * AnimatedVertex::Size());
	float* vertexBuffer = MeshUtils::ConvertAnimatedVerticesToArray(this->vertices);

	this->vertexBuffer = new VertexBuffer(vertexBuffer, vertexBufferSize);
	this->vertexBuffer->SetLayout(bufferLayout);

	uint32_t indexBufferSize = (uint32_t)(this->faces.size() * sizeof(Face));
	uint32_t* indexBuffer = MeshUtils::ConvertIndicesToArray(this->faces);

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

void AnimatedMesh::LoadNodes(aiNode* node, const glm::mat4& parentTransform)
{
	glm::mat4 transform = parentTransform * MeshUtils::ConvertToGLMMat4(node->mTransformation);
	this->nodeMap[node].resize(node->mNumMeshes);
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		unsigned int meshIndex = node->mMeshes[i];
		Submesh& submesh = this->submeshes[meshIndex];
		submesh.nodeName = node->mName.C_Str();
		submesh.transform = transform;
		this->nodeMap[node][i] = meshIndex;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		LoadNodes(node->mChildren[i], transform);
	}
}

void AnimatedMesh::SetupMaterials()
{
	// TODO: Setup materials
}