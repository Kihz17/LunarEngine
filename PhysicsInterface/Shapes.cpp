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
}
