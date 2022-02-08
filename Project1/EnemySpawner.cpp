#include "EnemySpawner.h"
#include "GLCommon.h"
#include "Utils.h"
#include "Steering.h"
#include "ApproachShootCondition.h"
#include "SeekCondition.h"
#include "FleeCondition.h"
#include "WanderCondition.h"
#include "IdleCondition.h"
#include "EvadeCondition.h"
#include "PursueCondition.h"
#include "Components.h"
#include "AILayer.h"

#include <glm/gtx/rotate_vector.hpp>
#include <Shapes.h>
#include <iostream>

bool test = false;

EnemySpawner::EnemySpawner(float spawnInterval, float spawnRadius, EntityManager& entityManager,
	Physics::IPhysicsFactory<Entity>* physicsFactory, Physics::IPhysicsWorld<Entity>* physicsWorld, AILayer* aiLayer, 
	Mesh* typeAMesh, Mesh* typeBMesh, Mesh* typeCMesh, Mesh* typeDMesh, Mesh* sphere)
	: spawnInterval(spawnInterval),
	spawnRadius(spawnRadius),
	entityManager(entityManager),
	physicsFactory(physicsFactory),
	physicsWorld(physicsWorld),
	aiLayer(aiLayer),
	lastSpawnTime(0.0f),
	typeAMesh(typeAMesh),
	typeBMesh(typeBMesh),
	typeCMesh(typeCMesh),
	typeDMesh(typeDMesh),
	sphere(sphere)
{

}

EnemySpawner::~EnemySpawner()
{

}

void EnemySpawner::OnUpdate(float deltaTime)
{
	//if (test) return;
	float currentTime = glfwGetTime();
	if (currentTime - lastSpawnTime > spawnInterval) // Spawn an enemy
	{
		//test = true;
		lastSpawnTime = currentTime;
		int spawnType = Utils::RandInt(0, 3);
		if (spawnType == 0)
		{
			SpawnTypeA();
		}
		else if (spawnType == 1)
		{
			SpawnTypeB();
		}
		else if (spawnType == 2)
		{
			SpawnTypeC();
		}
		else if (spawnType == 3)
		{
			SpawnTypeD();
		}
	}
}

void EnemySpawner::SpawnTypeA()
{
	Entity* typeA = SpawnPhysicsSphere(glm::vec3(0.0f, 0.9f, 0.0f), typeAMesh); // Green
	SteeringBehaviourComponent* typeAComp = typeA->AddComponent<SteeringBehaviourComponent>();
	typeAComp->AddTargetingBehaviour(0, new SeekCondition(aiLayer->CreateBehaviour<SeekBehaviour>(typeA->GetComponent<RigidBodyComponent>()->ptr, 1.0f, SeekType::None, false, 40.0f, 1.0f, 100.0f)));
	typeAComp->AddTargetingBehaviour(1, new FleeCondition(aiLayer->CreateBehaviour<FleeBehaviour>(typeA->GetComponent<RigidBodyComponent>()->ptr, 40.0f, 1.0f, 100.0f)));
}

void EnemySpawner::SpawnTypeB()
{
	Entity* typeB = SpawnPhysicsSphere(glm::vec3(0.0f, 0.0f, 0.6f), typeBMesh); // Blue
	SteeringBehaviourComponent* typeBComp = typeB->AddComponent<SteeringBehaviourComponent>();
	typeBComp->AddTargetingBehaviour(0, new EvadeCondition(aiLayer->CreateBehaviour<EvadeBehaviour>(typeB->GetComponent<RigidBodyComponent>()->ptr, glm::radians(10.0f), 60.0f, 1.0f, 100.0f)));
	typeBComp->AddTargetingBehaviour(1, new PursueCondition(aiLayer->CreateBehaviour<PursueBehaviour>(typeB->GetComponent<RigidBodyComponent>()->ptr, 1.0f, 40.0f, 1.0f, 100.0f)));
}

void EnemySpawner::SpawnTypeC()
{
	Entity* typeC = SpawnPhysicsSphere(glm::vec3(0.6f, 0.0f, 0.0f), typeCMesh); // Red
	SteeringBehaviourComponent* typeCComp = typeC->AddComponent<SteeringBehaviourComponent>();
	typeCComp->AddTargetingBehaviour(0, new ApproachShootCondition(aiLayer->CreateBehaviour<SeekBehaviour>(typeC->GetComponent<RigidBodyComponent>()->ptr, 30.0f, SeekType::Approach, true, 100.0f), 
		5.0f, entityManager, physicsFactory, physicsWorld, sphere));
}

void EnemySpawner::SpawnTypeD()
{
	Entity* typeD = SpawnPhysicsSphere(glm::vec3(0.6f, 0.6f, 0.0f), typeDMesh); // Yellow
	SteeringBehaviourComponent* typeDComp = typeD->AddComponent<SteeringBehaviourComponent>();
	typeDComp->AddBehaviour(0, new WanderCondition(aiLayer->CreateBehaviour<WanderBehaviour>(typeD->GetComponent<RigidBodyComponent>()->ptr, 40.0f, 10.0f, 100.0f), 6.0f, 3.0f));
	typeDComp->AddBehaviour(1, new IdleCondition(aiLayer->CreateBehaviour<IdleBehaviour>(typeD->GetComponent<RigidBodyComponent>()->ptr), 3.0f, 6.0f));
}

Entity* EnemySpawner::SpawnPhysicsSphere(const glm::vec3& color, Mesh* mesh)
{
	constexpr float radius = 1.0f;
	Entity* physicsSphere = entityManager.CreateEntity(name);
	physicsSphere->AddComponent<PositionComponent>();
	physicsSphere->AddComponent<ScaleComponent>(glm::vec3(0.009f, 0.009f, 0.009f));
	physicsSphere->AddComponent<RotationComponent>();

	Physics::SphereShape* shape = new Physics::SphereShape(radius);

	TagComponent* tags = physicsSphere->AddComponent<TagComponent>();
	tags->AddTag("enemy");
	tags->AddTag("health", new TagValue<float>(Utils::RandFloat(1.0f, 50.0f)));

	// Rigid Body
	Physics::RigidBodyInfo rigidInfo;
	rigidInfo.linearDamping = 0.99f;
	rigidInfo.angularDamping = 0.001f;
	rigidInfo.isStatic = false;
	rigidInfo.mass = radius;
	rigidInfo.position = glm::rotateY(Utils::FrontVec(), glm::radians(Utils::RandFloat(0.0f, 360.0f))) * spawnRadius; // Spawn at edge of map
	rigidInfo.position.y = 1.0f;
	rigidInfo.linearVelocity = glm::vec3(0.0f);
	rigidInfo.restitution = 0.8f;
	Physics::IRigidBody* rigidBody = physicsFactory->CreateRigidBody(rigidInfo, shape);
	physicsSphere->AddComponent<RigidBodyComponent>(rigidBody);
	physicsWorld->AddRigidBody(physicsSphere->GetComponent<RigidBodyComponent>()->ptr, physicsSphere);

	// Render Info
	RenderComponent::RenderInfo sphereInfo;
	sphereInfo.vao = mesh->GetVertexArray();
	sphereInfo.indexCount = mesh->GetIndexBuffer()->GetCount();
	sphereInfo.isColorOverride = true;
	sphereInfo.colorOverride = color;
	physicsSphere->AddComponent<RenderComponent>(sphereInfo);

	return physicsSphere;
}