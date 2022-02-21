#include "IMesh.h"
#include "Vertex.h"
#include "AnimatedVertex.h"

#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>


const uint32_t MeshUtils::ASSIMP_FLAGS =
	aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
	aiProcess_Triangulate |             // Make sure we're triangles
	aiProcess_SortByPType |             // Split meshes by primitive type
	aiProcess_GenNormals |              // Make sure we have legit normals
	aiProcess_GenUVCoords |             // Convert UVs if required 
	aiProcess_OptimizeMeshes |          // Batch draws where possible
	aiProcess_JoinIdenticalVertices |	// Join up identical vertices
	aiProcess_ValidateDataStructure;    // Validation

glm::mat4 MeshUtils::ConvertToGLMMat4(const aiMatrix4x4& matrix)
{
	glm::mat4 glmMat;
	glmMat[0][0] = matrix.a1;
	glmMat[1][0] = matrix.a2;
	glmMat[2][0] = matrix.a3;
	glmMat[3][0] = matrix.a4;
	glmMat[0][1] = matrix.b1;
	glmMat[1][1] = matrix.b2;
	glmMat[2][1] = matrix.b3;
	glmMat[3][1] = matrix.b4;
	glmMat[0][2] = matrix.c1;
	glmMat[1][2] = matrix.c2;
	glmMat[2][2] = matrix.c3;
	glmMat[3][2] = matrix.c4;
	glmMat[0][3] = matrix.d1;
	glmMat[1][3] = matrix.d2;
	glmMat[2][3] = matrix.d3;
	glmMat[3][3] = matrix.d4;
	return glmMat;
}

float* MeshUtils::ConvertVerticesToArray(const std::vector<IVertex*>& vertices)
{
	uint32_t vertexBufferSize = (uint32_t)(vertices.size() * Vertex::Length());
	float* vertexBuffer = new float[vertexBufferSize];
	uint32_t bufferIndex = 0;
	for (int i = 0; i < vertices.size(); i++) // Convert data into contiguous float array 
	{
		IVertex* vertex = vertices[i];
		float* data = vertex->Data();

		for (unsigned int j = 0; j < Vertex::Length(); j++)
		{
			vertexBuffer[bufferIndex + j] = data[j];
		}

		bufferIndex += Vertex::Length();
	}

	return vertexBuffer;
}

float* MeshUtils::ConvertAnimatedVerticesToArray(const std::vector<IVertex*>& vertices)
{
	uint32_t vertexBufferSize = (uint32_t)(vertices.size() * AnimatedVertex::Length());
	float* vertexBuffer = new float[vertexBufferSize];
	uint32_t bufferIndex = 0;
	for (int i = 0; i < vertices.size(); i++) // Convert data into contiguous float array 
	{
		IVertex* vertex = vertices[i];
		float* data = vertex->Data();

		for (unsigned int j = 0; j < AnimatedVertex::Length(); j++)
		{
			vertexBuffer[bufferIndex + j] = data[j];
		}

		bufferIndex += AnimatedVertex::Length();
	}

	return vertexBuffer;
}


uint32_t* MeshUtils::ConvertIndicesToArray(const std::vector<Face>& faces)
{
	uint32_t indexBufferSize = (uint32_t)(faces.size() * sizeof(Face));
	uint32_t* indexBuffer = new uint32_t[indexBufferSize];
	uint32_t bufferIndex = 0;
	for (int i = 0; i < faces.size(); i++) // Convert data into contiguous int array 
	{
		const Face& face = faces[i];
		indexBuffer[bufferIndex] = face.v1;
		indexBuffer[bufferIndex + 1] = face.v2;
		indexBuffer[bufferIndex + 2] = face.v3;

		bufferIndex += 3;
	}

	return indexBuffer;
}
