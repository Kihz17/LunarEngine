#include "IdleBehaviour.h"

IdleBehaviour::IdleBehaviour(Physics::IRigidBody* rigidBody)
	: SteeringBehaviour(rigidBody, SteeringBehaviourType::Normal, 0.0f, 0.0f, 0.0f)
{

}

IdleBehaviour::~IdleBehaviour()
{

}

void IdleBehaviour::Update(float deltaTime)
{

}

glm::vec3 IdleBehaviour::ComputeSteeringForce()
{
	return glm::vec3(0.0f);
}
