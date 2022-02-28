#include "CollisionHandler.h"
#include "RigidBody.h"
#include "Entity.h"
#include "PhysicsWorld.h"
#include "CollisionTesting.h"

#include <ICollisionListener.h>
#include <iostream>

glm::vec3 ClosestPointToPlane(const glm::vec3& point, const glm::vec3& normal, float dotProduct)
{
	float t = glm::dot(normal, point) - dotProduct;
	return point - t * normal;
}

glm::vec3 ProjectOn(glm::vec3 b, glm::vec3 a)
{
	float mult = ((a.x * b.x) + (a.y * b.y) + (a.z * b.z)) / ((b.x * b.x) + (b.y * b.y) + (b.z * b.z));
	return glm::vec3(b.x * mult, b.y * mult, b.z * mult);
}

bool CollisionHandler::CollideSphereSphere(RigidBody* bodyA, Physics::SphereShape* sphereA, 
	RigidBody* bodyB, Physics::SphereShape* sphereB, 
	float deltaTime, std::vector<Physics::ICollisionListener<Entity>*>& listeners, PhysicsWorld* physicsWorld)
{
	if (bodyA->IsStatic() && bodyB->IsStatic()) return false;

	glm::vec3 velocityA = bodyA->position - bodyA->previousPosition;
	glm::vec3 velocityB = bodyB->position - bodyB->previousPosition;

	float t = 0.0f;
	if (!CollisionTesting::TestSphereSphere(bodyA->previousPosition, sphereA->GetRadius(), bodyB->previousPosition, sphereB->GetRadius(), velocityA, velocityB, t)) return false; // No current or future collision

	if (t > deltaTime) return false; // No collision at this current step
	Physics::ICollisionListener<Entity>::CollisionEvent collisionEvent;
	collisionEvent.type = Physics::CollisionType::SphereSphere;
	collisionEvent.bodyA = bodyA;
	collisionEvent.bodyB = bodyB;
	collisionEvent.physicsWorld = physicsWorld;
	for (Physics::ICollisionListener<Entity>* listener : listeners) listener->Collide(collisionEvent);

	if (collisionEvent.isCancelled) return false;

	float fractDt = t / (glm::length(velocityA) + glm::length(velocityB));
	fractDt = glm::clamp(fractDt, 0.0f, 1.0f);
	float revDt = (1.0f - fractDt) * deltaTime;
	bodyA->VerletStep1(-revDt);
	bodyB->VerletStep1(-revDt);

	float totalMass = bodyA->mass + bodyB->mass;

	glm::vec3 positionDiff = bodyB->position - bodyA->position;
	float positionDiffLength = glm::length(positionDiff);
	float overlap = positionDiffLength - (sphereA->GetRadius() + sphereB->GetRadius());

	float factorA = bodyB->mass / totalMass;
	float factorB = bodyA->mass / totalMass;

	if (overlap < std::numeric_limits<float>::epsilon())
	{
		glm::vec3 overlapVec = positionDiff;
		overlapVec = glm::normalize(overlapVec);
		overlapVec *= -overlap;

		// If objects aren't static, move them to a position where they are not overlapping
		if (!bodyA->isStatic) bodyB->position += overlapVec * factorB;
		if (!bodyB->isStatic) bodyA->position -= overlapVec * factorA;

		positionDiff = bodyB->position - bodyA->position;
		positionDiffLength = glm::length(positionDiff);
	}

	positionDiff /= positionDiffLength;

	glm::vec3 momentumA = bodyA->linearVelocity * bodyA->mass;
	glm::vec3 momentumB = bodyB->linearVelocity * bodyB->mass;
	glm::vec3 momentumSum = momentumA + momentumB;

	momentumA = momentumSum * factorA;
	momentumB = momentumSum * factorB;

	constexpr float elasticity = 0.4f; // Ranges from 0.0 - 1.0

	glm::vec3 elasticMomentumB = positionDiff * (glm::length(momentumB) * elasticity) * -1.0f;
	glm::vec3 inelasticMomentumB = positionDiff * glm::length(momentumB) * (1.0f - elasticity);

	glm::vec3 elasticMomentumA = positionDiff * (glm::length(momentumA) * elasticity);
	glm::vec3 inelasticMomentumA = positionDiff * glm::length(momentumA) * (1.0f - elasticity);

	bodyA->linearVelocity -= (elasticMomentumA + inelasticMomentumA) * (bodyA->inverseMass * bodyA->restitution);
	bodyB->linearVelocity += (elasticMomentumB + inelasticMomentumB) * (bodyB->inverseMass * bodyB->restitution);

	bodyA->VerletStep1(revDt);
	bodyB->VerletStep1(revDt);

	return true;
}

bool CollisionHandler::CollideSpherePlane(RigidBody* bodyA, Physics::SphereShape* sphere, 
	RigidBody* bodyB, Physics::PlaneShape* plane, 
	float deltaTime, std::vector<Physics::ICollisionListener<Entity>*>& listeners, PhysicsWorld* physicsWorld)
{
	if (!CollisionTesting::TestSpherePlane(bodyA->previousPosition, bodyA->position, sphere->GetRadius(), plane->GetNormal(), plane->GetDotProduct())) return false;

	Physics::ICollisionListener<Entity>::CollisionEvent collisionEvent;
	collisionEvent.type = Physics::CollisionType::SpherePlane;
	collisionEvent.bodyA = bodyA;
	collisionEvent.bodyB = bodyB;
	collisionEvent.physicsWorld = physicsWorld;
	for (Physics::ICollisionListener<Entity>* listener : listeners) listener->Collide(collisionEvent);

	if (collisionEvent.isCancelled) return false;
	glm::vec3 closestPoint = ClosestPointToPlane(bodyA->position, plane->GetNormal(), plane->GetDotProduct());
	glm::vec3 overlapVec = closestPoint - bodyA->position;
	float overlapLength = glm::length(overlapVec);
	float velocityLength = glm::length(bodyA->linearVelocity);

	if (velocityLength > 0.000001f)
	{
		float overlapLength = glm::length(overlapVec);
		float velocity = glm::length(bodyA->linearVelocity);

		float fractDt = sphere->GetRadius() * ((sphere->GetRadius() / overlapLength) - 1.0f) / velocity; // Overlap ratio of the current timestep
		float partialDt = (1.0f - fractDt) * deltaTime; // The portion of delta time contributed to the overlap
		bodyA->VerletStep1(-partialDt); // Resolve sphere contact so that the sphere and plane are just touching

		glm::vec3 reflect = glm::reflect(bodyA->linearVelocity, plane->GetNormal());
		glm::vec3 cachedVelocity = bodyA->linearVelocity;
		bodyA->linearVelocity = reflect;

		glm::vec3 impactComponent = ProjectOn(bodyA->linearVelocity, plane->GetNormal());
		glm::vec3 impactTangent = cachedVelocity - impactComponent;
		bodyA->ApplyForce(impactTangent * -1.0f * bodyA->mass * bodyB->friction);
		bodyA->VerletStep1(partialDt);

		closestPoint = ClosestPointToPlane(bodyA->position, plane->GetNormal(), plane->GetDotProduct());
		overlapVec = closestPoint - bodyA->position;
		overlapLength = glm::length(overlapVec);
		if (overlapLength < sphere->GetRadius())
		{
			bodyA->position += plane->GetNormal() * (sphere->GetRadius() - overlapLength);
			float velDotNorm = glm::dot(bodyA->linearVelocity, plane->GetNormal());
			if (velDotNorm < 0.0f)
			{
				bodyA->linearVelocity -= plane->GetNormal() * velDotNorm;
			}
		}
		else
		{
			bodyA->linearVelocity *= bodyA->restitution;
		}
	}

	return true;
}

void CollisionHandler::Collide(float deltaTime, std::vector<RigidBody*>& rigidBodies, std::vector<CollidingBodies>& collidingBodies, 
	std::vector<Physics::ICollisionListener<Entity>*>& listeners, PhysicsWorld* physicsWorld)
{
	int bodyCount = rigidBodies.size();
	for (int i = 0; i < bodyCount - 1; i++)
	{
		for (int j = i + 1; j < bodyCount; j++)
		{
			bool collision = false;

			RigidBody* bodyA = rigidBodies[i];
			RigidBody* bodyB = rigidBodies[j];

			Physics::IShape* shapeA = bodyA->GetShape();
			Physics::IShape* shapeB = bodyB->GetShape();

			if (shapeA->GetShapeType() == Physics::ShapeType::Sphere)
			{
				if (shapeB->GetShapeType() == Physics::ShapeType::Sphere)
				{
					if (CollideSphereSphere(bodyA, Physics::SphereShape::Cast(shapeA), bodyB, Physics::SphereShape::Cast(shapeB), deltaTime, listeners, physicsWorld))
					{
						collision = true;
					}
				}
				else if (shapeB->GetShapeType() == Physics::ShapeType::Plane)
				{
					if (CollideSpherePlane(bodyA,  Physics::SphereShape::Cast(shapeA), bodyB, Physics::PlaneShape::Cast(shapeB), deltaTime, listeners, physicsWorld))
					{
						collision = true;
					}
				}
			}
			else if (shapeA->GetShapeType() == Physics::ShapeType::Plane)
			{
				if (shapeB->GetShapeType() == Physics::ShapeType::Sphere)
				{
					if (CollideSpherePlane(bodyB, Physics::SphereShape::Cast(shapeB), bodyA, Physics::PlaneShape::Cast(shapeA), deltaTime, listeners, physicsWorld))
					{
						collision = true;
					}
				}
			}

			if (collision)
			{
				collidingBodies.push_back({ bodyA, bodyB });
			}
		}
	}
}