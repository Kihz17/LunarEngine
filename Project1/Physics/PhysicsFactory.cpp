#include "PhysicsFactory.h"
#include "PhysicsWorld.h"

std::unordered_map<IMesh*, Physics::MeshShape*> PhysicsFactory::meshCoilliders;

PhysicsFactory::PhysicsFactory()
	: IPhysicsFactory()
{

}

PhysicsFactory::~PhysicsFactory()
{
	std::unordered_map<IMesh*, Physics::MeshShape*>::iterator it = meshCoilliders.begin();
	while (it != meshCoilliders.end())
	{
		delete it->second;
		it++;
	}
}

Physics::IPhysicsWorld* PhysicsFactory::CreateWorld()
{
	return new PhysicsWorld();
}

Physics::IRigidBody* PhysicsFactory::CreateRigidBody(const Physics::RigidBodyInfo& info, Physics::IShape* shape)
{
	return new RigidBody(info, shape);
}

Physics::MeshShape* PhysicsFactory::GetMeshShape(IMesh* mesh)
{
	std::unordered_map<IMesh*, Physics::MeshShape*>::iterator it = meshCoilliders.find(mesh);
	if (it != meshCoilliders.end()) return it->second;

	std::vector<int> faces;
	unsigned int faceSize = mesh->GetFaces().size() * 3;
	faces.resize(faceSize);

	int i = 0;
	for (const Face& face : mesh->GetFaces())
	{
		faces[i] = face.v1;
		faces[i + 1] = face.v2;
		faces[i + 2] = face.v3;
		i += 3;
	}

	std::vector<float> vertices;
	int vertexSize = mesh->GetVertices().size() * 3;
	vertices.resize(vertexSize);
	i = 0;
	for (IVertex* v : mesh->GetVertices())
	{
		vertices[i] = v->Data()[0];
		vertices[i + 1] = v->Data()[1];
		vertices[i + 2] = v->Data()[2];
		i += 3;
	}

	Physics::MeshShape* meshShape = new Physics::MeshShape(faces, 3 * sizeof(int), vertices, 3 * sizeof(float));
	meshCoilliders.insert({ mesh, meshShape });
	return meshShape;
}