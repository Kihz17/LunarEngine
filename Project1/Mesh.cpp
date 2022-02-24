#include "Mesh.h"
#include "Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>

#include <iostream>

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

Mesh::Mesh(const std::string& filePath)
	: vertexArray(nullptr),
	vertexBuffer(nullptr),
	indexBuffer(nullptr),
	filePath(filePath),
	boundingBox(nullptr)
{
	//AssimpLogger::Initialize();

	std::cout << "Loading mesh " << filePath << "..." << std::endl;;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filePath, MeshUtils::ASSIMP_FLAGS);
	if (!scene || !scene->HasMeshes())
	{
		std::cout << "Failed to load mesh file: " << filePath << std::endl;
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

	ParseNodes(scene->mRootNode);

	// Configure parent's bounding box based off of the submeshes we just added
	glm::vec3 parentMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 parentMax = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (Submesh& submesh : this->submeshes)
	{
		const glm::mat4& localTransform = submesh.localTransform;
		glm::vec3 min = submesh.minVertex;
		glm::vec3 max = submesh.maxVertex;

		parentMin.x = glm::min(parentMin.x, min.x);
		parentMin.y = glm::min(parentMin.y, min.y);
		parentMin.z = glm::min(parentMin.z, min.z);
		parentMax.x = glm::max(parentMax.x, max.x);
		parentMax.y = glm::max(parentMax.y, max.y);
		parentMax.z = glm::max(parentMax.z, max.z);
	}

	this->boundingBox = new AABB(parentMin, parentMax);

	BufferLayout bufferLayout = {
		{ ShaderDataType::Float3, "vPosition" },
		{ ShaderDataType::Float3, "vNormal" },
		{ ShaderDataType::Float2, "vTextureCoordinates" }
	};

	this->vertexArray = new VertexArrayObject();

	uint32_t vertexBufferSize = (uint32_t)(this->vertices.size() * Vertex::Size());
	float* vertexBuffer = MeshUtils::ConvertVerticesToArray(this->vertices);

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

Mesh::~Mesh()
{
	delete vertexArray;
	delete vertexBuffer;
	delete indexBuffer;

	for (IVertex* v : vertices) delete v;

	delete boundingBox;
}

void Mesh::ParseMesh(unsigned int meshIndex, const aiMesh* assimpMesh)
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

	if (!assimpMesh->HasTextureCoords(0))
	{
		std::cout << filePath << " does not have texture coords!" << std::endl;
	}

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

		vertices.push_back(new Vertex(pos, normal, texCoords));
	}

	submeshes[meshIndex].minVertex = minVertex;
	submeshes[meshIndex].maxVertex = maxVertex;

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

void Mesh::ParseMaterials(const aiScene* assimpScene)
{
	// TODO
}

void Mesh::ParseNodes(aiNode* node, const glm::mat4& parentTransform)
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