#pragma once

#include <Bullet/btBulletDynamicsCommon.h>

#include "IMesh.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Shapes.h>

#include <unordered_map>

class BulletUtils
{
public:
	static glm::vec3 BulletVec3ToGLM(const btVector3& v);
	static glm::quat BulletQuatToGLM(const btQuaternion& q);

	static btVector3 GLMVec3ToBullet(const glm::vec3& v);
	static btQuaternion GLMQuatToBullet(const glm::quat& q);

	static btMatrix3x3 GLMMat3ToBullet(const glm::mat3& m);
	static btTransform GLMMat4ToBullet(const glm::mat4& m);

	static btCollisionShape* ToBulletShape(Physics::IShape* shape);

private:
	static btBvhTriangleMeshShape* MakeMeshShape(Physics::MeshShape* shape);

	static std::unordered_map<Physics::IShape*, btBvhTriangleMeshShape*> meshBulletShapes;
};