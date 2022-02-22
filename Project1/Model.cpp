#include "Model.h"
#include "Vertex.h"
#include "MeshManager.h"

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

Model::Model(const std::string& filePath)
	: filePath(filePath), 
	boundingBox(nullptr)
{
	//AssimpLogger::Initialize();

	std::cout << "Loading model " << filePath << "..." << std::endl;;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filePath, MeshUtils::ASSIMP_FLAGS);
	if (!scene || !scene->HasMeshes())
	{
		std::cout << "Failed to load model file: " << filePath << std::endl;
		return;
	}

	this->submeshes.reserve(scene->mNumMeshes);

	unsigned int vertexCount = 0;
	unsigned int indexCount = 0;
	ParseNode(scene->mRootNode, scene, glm::mat4(1.0f));

	// Configure parent's bounding box based off of the submeshes we just added
	glm::vec3 parentMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 parentMax = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (Submesh& submesh : this->submeshes)
	{
		const AABB* submeshAABB = submesh.mesh->GetBoundingBox();
		glm::vec3 submeshMin = submeshAABB->GetMin();
		glm::vec3 submeshMax = submeshAABB->GetMax();

		const glm::mat4& localTransform = submesh.localTransform;
		glm::vec3 min = glm::vec3(localTransform * glm::vec4(submeshMin, 1.0f));
		glm::vec3 max = glm::vec3(localTransform * glm::vec4(submeshMax, 1.0f));

		parentMin.x = glm::min(parentMin.x, min.x);
		parentMin.y = glm::min(parentMin.y, min.y);
		parentMin.z = glm::min(parentMin.z, min.z);
		parentMax.x = glm::max(parentMax.x, max.x);
		parentMax.y = glm::max(parentMax.y, max.y);
		parentMax.z = glm::max(parentMax.z, max.z);
	}

	this->boundingBox = new AABB(parentMin, parentMax);
}

Model::~Model()
{
	delete boundingBox;
}

void Model::ParseNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform)
{
	glm::mat4 transform = parentTransform * MeshUtils::ConvertToGLMMat4(node->mTransformation);

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];

		IMesh* mesh = MeshManager::LoadMesh(filePath, assimpMesh, node->mMeshes[i]);

		// TODO: Load materials into textures

		Submesh submesh;
		submesh.mesh = mesh;
		submesh.localTransform = transform;
		this->submeshes.push_back(submesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ParseNode(node->mChildren[i], scene, transform);
	}
}