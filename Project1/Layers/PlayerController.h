#pragma once

#include "ApplicationLayer.h"
#include "EntityManager.h"
#include "Camera.h"
#include "RigidBody.h"
#include "PhysicsWorld.h"
#include "ASM.h"

#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

struct AttackStage
{
	AnimationState& animationState;
	float chainingWindow = 0.2f; // Represents the time in seconds that will be considered for animation chaining
	float impulse = 0.0f; // TODO: Implement "forces" on ability use
};

struct Attack
{
	int currentStage = 0;
	std::vector<AttackStage> attackStages;
};

class SkeletalAnimationComponent;
class Animation;
class PlayerController : public ApplicationLayer
{
public:
	PlayerController(Camera& camera, EntityManager& entityManager, PhysicsWorld* physicsWorld);

	virtual void OnAttach() override;
	virtual void OnUpdate(float deltaTime) override;

private:
	EntityManager& entityManager;
	PhysicsWorld* physicsWorld;
	Camera& camera;

	Entity* playerEntity;
	btKinematicCharacterController* btController;
	btPairCachingGhostObject* ghostObj;

	bool equipped;

	SkeletalAnimationComponent* animComp;
	ASM animationStateMachine;

	AnimationState unequipIdle;
	AnimationState unequipWalk;
	AnimationState unequipRun;
	AnimationState jump;

	AnimationState equip;
	AnimationState unequip;

	AnimationState equipIdle;
	AnimationState equipRunF;
	AnimationState equipRunFR;
	AnimationState equipRunFL;
	AnimationState equipRunL;
	AnimationState equipRunR;
	AnimationState equipRunB;
	AnimationState equipRunBR;
	AnimationState equipRunBL;

	AnimationState equipWalkF;
	AnimationState equipWalkFR;
	AnimationState equipWalkFL;
	AnimationState equipWalkL;
	AnimationState equipWalkR;
	AnimationState equipWalkB;
	AnimationState equipWalkBR;
	AnimationState equipWalkBL;

	AnimationState attack1;
	AnimationState attack2;
	AnimationState attack3;
	AnimationState attack4;

	Attack basicAttack;
	float lastLeftClickTime;
};