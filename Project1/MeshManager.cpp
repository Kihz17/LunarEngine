#include "MeshManager.h"
#include "Vertex.h"
#include "Mesh.h"

std::map<std::string, std::map<unsigned int, IMesh*>> MeshManager::loadedMeshes;

IMesh* MeshManager::LoadMesh(const std::string& filePath, aiMesh* assimpMesh, unsigned int assimpMeshIndex)
{
	if (loadedMeshes.find(filePath) == loadedMeshes.end()) // This model hasn't been loaded yet
	{
		loadedMeshes[filePath] = std::map<unsigned int, IMesh*>();
	}

	std::map<unsigned int, IMesh*>& meshMap = loadedMeshes[filePath];
	if (meshMap.find(assimpMeshIndex) != meshMap.end()) // Mesh has already been loaded
	{
		return meshMap[assimpMeshIndex];
	}

	if (!assimpMesh->HasPositions())
	{
		std::cout << filePath << " does not have position!" << std::endl;
		return nullptr;
	}

	if (!assimpMesh->HasNormals())
	{
		std::cout << filePath << " does not have normals!" << std::endl;
		return nullptr;
	}

	// Define min/max for this meshes bounding box
	glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	std::vector<IVertex*> vertices;
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

		vertices.push_back(new Vertex(position, normal, textureCoords));
	}

	AABB* submeshBoundingBox = new AABB(min, max); // Resize the bounding box to the proper size

	// Assign faces/indices
	std::vector<Face> faces;
	for (unsigned int j = 0; j < assimpMesh->mNumFaces; j++)
	{
		if (assimpMesh->mFaces[j].mNumIndices != 3)
		{
			std::cout << "Face must be a triangle!" << std::endl;
			return nullptr;
		}

		Face face;
		face.v1 = assimpMesh->mFaces[j].mIndices[0];
		face.v2 = assimpMesh->mFaces[j].mIndices[1];
		face.v3 = assimpMesh->mFaces[j].mIndices[2];
		faces.push_back(face);
	}

	IMesh* newMesh = new Mesh(vertices, faces, submeshBoundingBox);
	meshMap.insert({ assimpMeshIndex, newMesh });
	return newMesh;
}