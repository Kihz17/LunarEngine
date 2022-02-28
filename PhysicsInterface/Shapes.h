#pragma once 

#include "IShape.h"

#include <glm/glm.hpp>

namespace Physics
{
	class SphereShape : public IShape
	{
	public:
		SphereShape(float radius);
		virtual ~SphereShape();

		float GetRadius() const { return radius; }

		static SphereShape* Cast(IShape* shape);

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
		PlaneShape(float dotProduct, const glm::vec3& normal);
		virtual ~PlaneShape();

		float GetDotProduct() const { return dotProduct; }
		const glm::vec3& GetNormal() { return normal; }

		static PlaneShape* Cast(IShape* shape);

	private:
		PlaneShape(const PlaneShape& other) : IShape(ShapeType::Plane) {}
		PlaneShape& operator=(const PlaneShape& other) { return *this; }

		float dotProduct;
		glm::vec3 normal;
	};

	//class AABBShape : public IShape
	//{
	//public:
	//	AABBShape();
	//	virtual ~AABBShape();

	//	static AABBShape* Cast(IShape* shape);

	//private:
	//	AABBShape(const AABBShape& other) : IShape(ShapeType::AABB) {}
	//	AABBShape& operator=(const AABBShape& other) { return *this; }
	//};

	//class OBBShape : public IShape
	//{
	//public:
	//	OBBShape();
	//	virtual ~OBBShape();

	//	static OBBShape* Cast(IShape* shape);

	//private:
	//	OBBShape(const OBBShape& other) : IShape(ShapeType::OBB) {}
	//	OBBShape& operator=(const OBBShape& other) { return *this; }
	//};

	//class CapsuleShape : public IShape
	//{
	//public:
	//	CapsuleShape();
	//	virtual ~CapsuleShape();

	//	static CapsuleShape* Cast(IShape* shape);

	//private:
	//	CapsuleShape(const CapsuleShape& other) : IShape(ShapeType::Capsule) {}
	//	CapsuleShape& operator=(const CapsuleShape& other) { return *this; }
	//};
}