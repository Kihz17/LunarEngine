#include "BulletUtils.h"

#include <iostream>

std::unordered_map<Physics::IShape*, btBvhTriangleMeshShape*> BulletUtils::meshBulletShapes;

glm::vec3 BulletUtils::BulletVec3ToGLM(const btVector3& v)
{
	return glm::vec3(v.x(), v.y(), v.z());
}

glm::quat BulletUtils::BulletQuatToGLM(const btQuaternion& q)
{
	return glm::quat(q.w(), q.x(), q.y(), q.z());
}

btVector3 BulletUtils::GLMVec3ToBullet(const glm::vec3& v)
{
	return btVector3(v.x, v.y, v.z);
}

btQuaternion BulletUtils::GLMQuatToBullet(const glm::quat& q)
{
	return btQuaternion(q.x, q.y, q.z, q.w);
}

btMatrix3x3 BulletUtils::GLMMat3ToBullet(const glm::mat3& m)
{
	btMatrix3x3 mat;
	mat.setRotation(GLMQuatToBullet(glm::toQuat(m)));
	return mat;
}

btTransform BulletUtils::GLMMat4ToBullet(const glm::mat4& m)
{
	btTransform t;
	t.setRotation(GLMQuatToBullet(glm::toQuat(m)));
	t.setOrigin(GLMVec3ToBullet(glm::vec3(m[3])));
	return t;
}

btCollisionShape* BulletUtils::ToBulletShape(Physics::IShape* shape)
{
	switch (shape->GetShapeType())
	{
	case Physics::ShapeType::Box:
	{
		Physics::BoxShape* box = Physics::BoxShape::Cast(shape);
		return new btBoxShape(GLMVec3ToBullet(box->GetExtents()));
	}
		break;

	case Physics::ShapeType::Sphere:
	{
		Physics::SphereShape* sphere = Physics::SphereShape::Cast(shape);
		return new btSphereShape(sphere->GetRadius());
	}
		break;

	case Physics::ShapeType::Plane:
	{
		Physics::PlaneShape* plane = Physics::PlaneShape::Cast(shape);
		return new btStaticPlaneShape(GLMVec3ToBullet(plane->GetNormal()), plane->GetDotProduct());
		break;
	}
		
	case Physics::ShapeType::Cylinder:
	{
		Physics::CylinderShape* cylinder = Physics::CylinderShape::Cast(shape);
		return new btCylinderShape(GLMVec3ToBullet(cylinder->GetExtents()));
	}
		break;

	case Physics::ShapeType::Capsule:
	{
		Physics::CapsuleShape* capsule = Physics::CapsuleShape::Cast(shape);
		return new btCapsuleShape(capsule->GetRadius(), capsule->GetHeight());
	}
		break;

	case Physics::ShapeType::Mesh:
	{
		std::unordered_map<Physics::IShape*, btBvhTriangleMeshShape*>::iterator it = meshBulletShapes.find(shape);
		if (it != meshBulletShapes.end()) return it->second;

		Physics::MeshShape* meshShape = Physics::MeshShape::Cast(shape);
		btBvhTriangleMeshShape* bulletMeshShape = MakeMeshShape(meshShape);

		meshBulletShapes.insert({ shape, bulletMeshShape });
		return bulletMeshShape;
	}	
		break;

	case Physics::ShapeType::ScaledMesh:
	{
		Physics::ScaledMeshShape* scaledMesh = Physics::ScaledMeshShape::Cast(shape);

		std::unordered_map<Physics::IShape*, btBvhTriangleMeshShape*>::iterator it = meshBulletShapes.find(scaledMesh->GetMeshShape());
		if (it != meshBulletShapes.end()) return new btScaledBvhTriangleMeshShape(it->second, GLMVec3ToBullet(scaledMesh->GetScale()));

		Physics::MeshShape* meshShape = scaledMesh->GetMeshShape();
		btBvhTriangleMeshShape* bulletMeshShape = MakeMeshShape(meshShape);
		meshBulletShapes.insert({ scaledMesh->GetMeshShape(), bulletMeshShape });

		return new btScaledBvhTriangleMeshShape(bulletMeshShape, GLMVec3ToBullet(scaledMesh->GetScale()));
	}
		break;

	default:
		return nullptr;
	}
}


btBvhTriangleMeshShape* BulletUtils::MakeMeshShape(Physics::MeshShape* shape)
{
	const std::vector<int>& faces = shape->GetFaces();
	const std::vector<float>& vertices = shape->GetVertices();

	int* fs = new int[faces.size()];
	for (int i = 0; i < faces.size(); i++) fs[i] = faces[i];

	float* vs = new float[vertices.size()];
	for (int i = 0; i < vertices.size(); i++) vs[i] = vertices[i];

	btTriangleIndexVertexArray* vertexData = new btTriangleIndexVertexArray(faces.size() / 3, fs, shape->GetFaceStride(), vertices.size() / 3, vs, shape->GetVertexStride());

	return new btBvhTriangleMeshShape(vertexData, true);
}