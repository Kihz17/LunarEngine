#pragma once

#include "Mesh.h"
#include "BulletUtils.h"

#include <Bullet/btBulletDynamicsCommon.h>

#include <glm/glm.hpp>

#include <unordered_map>

enum class MeshColliderType
{
	Triangle,
	BvhTriangle,
	ConvexTriangle
};

// TODO: Track bullet collision shapes to not duplicate them when making new rigidbodies

class PhysicsFactory
{
public:
	static btCollisionShape* GetMeshShape(IMesh* mesh, MeshColliderType type);
	static btCollisionShape* GetMeshScaledShape(IMesh* mesh, const glm::vec3& scale);

private:
	static std::unordered_map<IMesh*, std::unordered_map<MeshColliderType, btCollisionShape*>> meshCoilliders;
};