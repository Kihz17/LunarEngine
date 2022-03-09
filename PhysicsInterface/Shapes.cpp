#include "Shapes.h"

namespace Physics
{
	/////////////////
	// SPHERE
	/////////////////
	SphereShape::SphereShape(float radius)
		: IShape(ShapeType::Sphere),
		radius(radius)
	{

	}

	SphereShape::~SphereShape()
	{

	}

	SphereShape* SphereShape::Cast(IShape* shape)
	{
		return dynamic_cast<SphereShape*>(shape);
	}

	/////////////////
	// PLANE
	/////////////////
	PlaneShape::PlaneShape(float dotProduct, const glm::vec3& normal)
		: IShape(ShapeType::Plane),
		dotProduct(dotProduct),
		normal(normal)
	{

	}

	PlaneShape::~PlaneShape()
	{

	}

	PlaneShape* PlaneShape::Cast(IShape* shape)
	{
		return dynamic_cast<PlaneShape*>(shape);
	}

	/////////////////
	// AABB
	/////////////////
	AABBShape::AABBShape(const glm::vec3& size)
		: IShape(ShapeType::AABB),
		size(size)
	{

	}

	AABBShape::~AABBShape()
	{

	}

	AABBShape* AABBShape::Cast(IShape* shape)
	{
		return dynamic_cast<AABBShape*>(shape);
	}

	/////////////////
	// OBB
	/////////////////
	OBBShape::OBBShape(const glm::vec3& size)
		: IShape(ShapeType::OBB),
		size(size)
	{

	}

	OBBShape::~OBBShape()
	{

	}

	OBBShape* OBBShape::Cast(IShape* shape)
	{
		return dynamic_cast<OBBShape*>(shape);
	}

	/////////////////
	// CAPSULE
	/////////////////
}
