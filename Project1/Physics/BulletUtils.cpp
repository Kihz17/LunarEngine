#include "BulletUtils.h"

namespace BulletUtils
{
	glm::vec3 BulletVec3ToGLM(const btVector3& v)
	{
		return glm::vec3(v.x(), v.y(), v.z());
	}

	glm::quat BulletQuatToGLM(const btQuaternion& q)
	{
		return glm::quat(q.w(), q.x(), q.y(), q.z());
	}

	btVector3 GLMVec3ToBullet(const glm::vec3& v)
	{
		return btVector3(v.x, v.y, v.z);
	}

	btQuaternion GLMQuatToBullet(const glm::quat& q)
	{
		return btQuaternion(q.w, q.x, q.y, q.z);
	}

	btMatrix3x3 GLMMat3ToBullet(const glm::mat3& m)
	{
		btMatrix3x3 mat;
		mat.setRotation(GLMQuatToBullet(glm::toQuat(m)));
		return mat;
	}

	btTransform GLMMat4ToBullet(const glm::mat4& m)
	{
		btTransform t;
		t.setRotation(GLMQuatToBullet(glm::toQuat(m)));
		t.setOrigin(GLMVec3ToBullet(glm::vec3(m[3])));
		return t;
	}
}