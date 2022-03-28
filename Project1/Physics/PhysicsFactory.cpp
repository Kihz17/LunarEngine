#include "PhysicsFactory.h"

std::unordered_map<IMesh*, std::unordered_map<MeshColliderType, btCollisionShape*>> PhysicsFactory::meshCoilliders;

btCollisionShape* PhysicsFactory::GetMeshShape(IMesh* mesh, MeshColliderType type)
{
	std::unordered_map<IMesh*, std::unordered_map<MeshColliderType, btCollisionShape*>>::iterator it = meshCoilliders.find(mesh);
	if (it != meshCoilliders.end())
	{
		std::unordered_map<MeshColliderType, btCollisionShape*>& map = it->second;

		std::unordered_map<MeshColliderType, btCollisionShape*>::iterator typeIt = map.find(type);
		if (typeIt != map.end())
		{
			return typeIt->second;
		}
	
		// TODO: Make and return;
		
	}

	const std::vector<Face>& faces = mesh->GetFaces();
	int faceSize = faces.size() * 3;
	int* triangles = new int[faceSize];
	int i = 0;
	for (const Face& f : faces)
	{
		triangles[i] = f.v1;
		triangles[i + 1] = f.v2;
		triangles[i + 2] = f.v3;
		i += 3;
	}

	const std::vector<IVertex*>& vertices = mesh->GetVertices();
	int vertexSize = vertices.size() * 3;
	float* verts = new float[vertexSize];
	i = 0;
	for (IVertex* v : vertices)
	{
		verts[i] = v->Data()[0];
		verts[i + 1] = v->Data()[1];
		verts[i + 2] = v->Data()[2];
		i += 3;
	}

	if (type == MeshColliderType::BvhTriangle)
	{
		btTriangleIndexVertexArray* vertexData = new btTriangleIndexVertexArray(faces.size(), triangles, 3 * sizeof(int), vertices.size(), verts, 3 * sizeof(float));
		btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(vertexData, true);
		meshCoilliders.insert({mesh, std::unordered_map<MeshColliderType, btCollisionShape*>()});
		meshCoilliders.at(mesh).insert({ type , shape});
		return shape;
	}
	
	return nullptr;
}

btCollisionShape* PhysicsFactory::GetMeshScaledShape(IMesh* mesh, const glm::vec3& scale)
{
	btBvhTriangleMeshShape* baseShape = nullptr;

	std::unordered_map<IMesh*, std::unordered_map<MeshColliderType, btCollisionShape*>>::iterator it = meshCoilliders.find(mesh);
	if (it != meshCoilliders.end())
	{
		std::unordered_map<MeshColliderType, btCollisionShape*>& map = it->second;
		std::unordered_map<MeshColliderType, btCollisionShape*>::iterator typeIt = map.find(MeshColliderType::BvhTriangle);
		if (typeIt != map.end())
		{
			baseShape = static_cast<btBvhTriangleMeshShape*>(typeIt->second);
		}
	}

	if (!baseShape)
	{
		const std::vector<Face>& faces = mesh->GetFaces();
		int faceSize = faces.size() * 3;
		int* triangles = new int[faceSize];
		int i = 0;
		for (const Face& f : faces)
		{
			triangles[i] = f.v1;
			triangles[i + 1] = f.v2;
			triangles[i + 2] = f.v3;
			i += 3;
		}

		const std::vector<IVertex*>& vertices = mesh->GetVertices();
		int vertexSize = vertices.size() * 3;
		float* verts = new float[vertexSize];
		i = 0;
		for (IVertex* v : vertices)
		{
			verts[i] = v->Data()[0];
			verts[i + 1] = v->Data()[1];
			verts[i + 2] = v->Data()[2];
			i += 3;
		}

		// Make base collider
		btTriangleIndexVertexArray* vertexData = new btTriangleIndexVertexArray(faces.size(), triangles, 3 * sizeof(int), vertices.size(), verts, 3 * sizeof(float));
		baseShape = new btBvhTriangleMeshShape(vertexData, true);

		meshCoilliders.insert({ mesh, std::unordered_map<MeshColliderType, btCollisionShape*>() });
		meshCoilliders.at(mesh).insert({ MeshColliderType::BvhTriangle , baseShape });
	}

	// Make scaled collider
	btScaledBvhTriangleMeshShape* scaledShape = new btScaledBvhTriangleMeshShape(baseShape, BulletUtils::GLMVec3ToBullet(scale));
	return scaledShape;
}