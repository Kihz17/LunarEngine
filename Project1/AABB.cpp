#include "AABB.h"
#include "VertexInformation.h"
#include "VertexArrayObject.h"
#include "IndexBuffer.h"

#include "GLCommon.h"

constexpr int numCubeVertices = 36;
constexpr int cubeVertexSize = 8; // x,y,z | nx, ny, nz | uv, uy
constexpr int cubeVertexArraySize = numCubeVertices * cubeVertexSize;

AABB::AABB()
	: center(0.0f),
	size(0.0f),
	vao(nullptr),
	vbo(nullptr),
	ebo(nullptr)
{
	
}

AABB::AABB(const glm::vec3& min, const glm::vec3 max) 
	: center((max + min) * 0.5f),
	size(glm::vec3(max.x - center.x, max.y - center.y, max.z - center.z)),
	vao(nullptr),
	vbo(nullptr),
	ebo(nullptr)
{
	SetupVertices();
}

AABB::AABB(const glm::vec3& center, float sizeX, float sizeY, float sizeZ)
	: center(center),
	size(glm::vec3(sizeX, sizeY, sizeZ)),
	vao(nullptr),
	vbo(nullptr),
	ebo(nullptr)
{
	SetupVertices();
}

AABB::~AABB()
{
	if(vao) delete vao;
	if(vbo) delete vbo;
	if(ebo) delete ebo;
}

bool AABB::IsOnFrustum(const Frustum& frustum, const glm::mat4& transform) const
{
	const glm::vec3 transformedCenter = transform * glm::vec4(center, 1.0f);

	// Scaled orientation
	const glm::vec3 right = glm::vec3(transform[0]) * size.x;
	const glm::vec3 up = glm::vec3(transform[1]) * size.y;
	const glm::vec3 forward = glm::vec3(-transform[2]) * size.z;

	const float newSizeX = std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), right)) +
		std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), up)) +
		std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), forward));

	const float newSizeY = std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), right)) +
		std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), up)) +
		std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), forward));

	const float newSizeZ = std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), right)) +
		std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), up)) +
		std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), forward));

	const AABB transformedBoundingBox(transformedCenter, newSizeX, newSizeY, newSizeZ);

	return transformedBoundingBox.IsOnOrForwardPlan(frustum.left) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.right) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.top) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.bottom) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.near) &&
		transformedBoundingBox.IsOnOrForwardPlan(frustum.far);
}

bool AABB::IsOnOrForwardPlan(const Plane& plane) const // Taken from https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
{
	// Compute projection interval radius of b onto L(t) = b.x * p.n
	const float r = size.x * std::abs(plane.normal.x) + size.y * std::abs(plane.normal.y) + size.z * std::abs(plane.normal.z);
	return -r < plane.GetSignedDistanceToPlan(center);
}

void AABB::Resize(const glm::vec3& min, const glm::vec3 max)
{
	center = (max + min) * 0.5f;
	size = glm::vec3(max.x - center.x, max.y - center.y, max.z - center.z);

	if (vao) delete vao;
	if (vbo) delete vbo;
	if (ebo) delete ebo;

	SetupVertices();
}

glm::vec3 AABB::GetMin() const
{
	return center - size;
}

glm::vec3 AABB::GetMax() const
{
	return center + size;
}

void AABB::SetupVertices()
{
	const glm::vec3 min = GetMin();
	const glm::vec3 max = GetMax();

	float vertexData[24];

	vertexData[0] = min.x;
	vertexData[1] = min.y;
	vertexData[2] = max.z;

	vertexData[3] = max.x;
	vertexData[4] = min.y;
	vertexData[5] = max.z;

	vertexData[6] = max.x;
	vertexData[7] = max.y;
	vertexData[8] = max.z;

	vertexData[9] = min.x;
	vertexData[10] = max.y;
	vertexData[11] = max.z;

	// Top
	vertexData[12] = min.x;
	vertexData[13] = min.y;
	vertexData[14] = min.z;

	vertexData[15] = max.x;
	vertexData[16] = min.y;
	vertexData[17] = min.z;

	vertexData[18] = max.x;
	vertexData[19] = max.y;
	vertexData[20] = min.z;

	vertexData[21] = min.x;
	vertexData[22] = max.y;
	vertexData[23] = min.z;

	vao = new VertexArrayObject();

	BufferLayout layout = {
		{ ShaderDataType::Float3, "vPosition" }
	};

	vbo = new VertexBuffer(vertexData, sizeof(float) * 24);
	vbo->SetLayout(layout);

	uint32_t indices[] = {
		0, 1, 1, 2, 2, 3, 3, 0, // Front
		4, 5, 5, 6, 6, 7, 7, 4, // Back
		0, 4, 1, 5, 2, 6, 3, 7
	};

	ebo = new IndexBuffer(indices, 24);

	vao->AddVertexBuffer(vbo);
	vao->SetIndexBuffer(ebo);
}