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
}