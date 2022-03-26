#pragma once

#include <Bullet/btBulletDynamicsCommon.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace BulletUtils
{
	glm::vec3 BulletVec3ToGLM(const btVector3& v);
	glm::quat BulletQuatToGLM(const btQuaternion& q);

	btVector3 GLMVec3ToBullet(const glm::vec3& v);
	btQuaternion GLMQuatToBullet(const glm::quat& q);

	btMatrix3x3 GLMMat3ToBullet(const glm::mat3& m);
	btTransform GLMMat4ToBullet(const glm::mat4& m);
}