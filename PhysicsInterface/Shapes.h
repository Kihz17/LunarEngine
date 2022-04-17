#pragma once 

#include "IShape.h"

#include <glm/glm.hpp>
#include <vector>

namespace Physics
{

	class BoxShape : public IShape
	{
	public:
		BoxShape(const glm::vec3& extents) : IShape(ShapeType::Box), extents(extents) {}
		virtual ~BoxShape() {}

		const glm::vec3& GetExtents() const { return extents; }

		static BoxShape* Cast(IShape* shape) { return dynamic_cast<BoxShape*>(shape); }

	private:
		BoxShape(const BoxShape& other) : IShape(ShapeType::Box) {}
		BoxShape& operator=(const BoxShape& other) { return *this; }

		glm::vec3 extents;
	};

	class SphereShape : public IShape
	{
	public:
		SphereShape(float radius) : IShape(ShapeType::Sphere), radius(radius) {}
		virtual ~SphereShape() {}

		float GetRadius() const { return radius; }

		static SphereShape* Cast(IShape* shape) { return dynamic_cast<SphereShape*>(shape); }

	protected:
		SphereShape(ShapeType shapeType): IShape(shapeType) {}

	private:
		SphereShape(const SphereShape& other) : IShape(ShapeType::Sphere) {}
		SphereShape& operator=(const SphereShape& other) { return *this; }

		float radius;
	};

	class PlaneShape : public IShape
	{
	public:
		PlaneShape(float dotProduct, const glm::vec3& normal) : IShape(ShapeType::Plane), dotProduct(dotProduct), normal(normal) {}
		virtual ~PlaneShape() {}

		float GetDotProduct() const { return dotProduct; }
		const glm::vec3& GetNormal() { return normal; }

		static PlaneShape* Cast(IShape* shape) { return dynamic_cast<PlaneShape*>(shape); }

	private:
		PlaneShape(const PlaneShape& other) : IShape(ShapeType::Plane) {}
		PlaneShape& operator=(const PlaneShape& other) { return *this; }

		float dotProduct;
		glm::vec3 normal;
	};

	class CylinderShape : public IShape
	{
	public:
		CylinderShape(const glm::vec3& halfExtents) : IShape(ShapeType::Cylinder), extents(halfExtents) {}
		virtual ~CylinderShape() {}

		const glm::vec3& GetExtents() const { return extents; }

		static CylinderShape* Cast(IShape* shape) { return dynamic_cast<CylinderShape*>(shape); }

	private:
		glm::vec3 extents;

		CylinderShape(const CylinderShape&) : IShape(ShapeType::Cylinder) { }
		CylinderShape& operator=(const CylinderShape&) { return *this; }
	};

	class ConeShape : public IShape
	{
	public:
		ConeShape(float radius, float height) : IShape(ShapeType::Cone), radius(radius), height(height) {}
		virtual ~ConeShape() {}

		float GetRadius() const { return radius; }
		float GetHeight() const { return height; }

		static ConeShape* Cast(IShape* shape) { return dynamic_cast<ConeShape*>(shape); }

	private:
		float radius;
		float height;

		ConeShape(const ConeShape&) : IShape(ShapeType::Cone) { }
		ConeShape& operator=(const ConeShape&) { return *this; }
	};

	class CapsuleShape : public IShape
	{
	public:
		CapsuleShape(float radius, float height) : IShape(ShapeType::Capsule), radius(radius), height(height) {}
		virtual ~CapsuleShape() {}

		float GetRadius() const { return radius; }
		float GetHeight() const { return height; }

		static CapsuleShape* Cast(IShape* shape) { return dynamic_cast<CapsuleShape*>(shape); }

	private:
		float radius;
		float height;

		CapsuleShape(const CapsuleShape& other) : IShape(ShapeType::Capsule) {}
		CapsuleShape& operator=(const CapsuleShape& other) { return *this; }
	};

	class MeshShape : public IShape
	{
	public:
		MeshShape(const std::vector<int>& faces, int faceStride, const std::vector<float>& vertices, int vertexStride)
			: IShape(ShapeType::Mesh), faces(faces), faceStride(faceStride), vertices(vertices), vertexStride(vertexStride) {}

		virtual ~MeshShape() {}

		const std::vector<int>& GetFaces() const { return faces; }
		const std::vector<float>& GetVertices() const { return vertices; }

		int GetFaceStride() const { return faceStride; }
		int GetVertexStride() const { return vertexStride; }

		static MeshShape* Cast(IShape* shape) { return dynamic_cast<MeshShape*>(shape); }

	private:
		std::vector<int> faces;
		int faceStride;

		std::vector<float> vertices;
		int vertexStride;

		MeshShape(const MeshShape& other) : IShape(ShapeType::Mesh) {}
		MeshShape& operator=(const MeshShape& other) { return *this; }
	};

	class ScaledMeshShape : public IShape
	{
	public:
		ScaledMeshShape(MeshShape* meshShape, const glm::vec3& scale) : IShape(ShapeType::ScaledMesh), baseShape(meshShape), scale(scale) {}
		virtual ~ScaledMeshShape() {}

		MeshShape* GetMeshShape() { return baseShape; }
		glm::vec3 GetScale() const { return scale; }

		static ScaledMeshShape* Cast(IShape* shape) { return dynamic_cast<ScaledMeshShape*>(shape); }

	private:
		MeshShape* baseShape;
		glm::vec3 scale;

		ScaledMeshShape(const ScaledMeshShape& other) : IShape(ShapeType::ScaledMesh) {}
		ScaledMeshShape& operator=(const ScaledMeshShape& other) { return *this; }
	};
}