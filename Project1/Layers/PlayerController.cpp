#include "PlayerController.h"
#include "Components.h"
#include "InputManager.h"
#include "MeshManager.h"

#include <glm/gtx/vector_angle.hpp>
#include <iostream>

constexpr float walkSpeed = 0.1f;
constexpr float runSpeed = 0.4f;
constexpr float equipRunSpeed = 0.3f;

constexpr float animIdleSpeed = 15.0f;
constexpr float animWalkSpeed = 15.0f;
constexpr float animSprintSpeed = 17.0f;
constexpr float animEquipSpeed = 19.0f;
constexpr float animJumpSpeed = 20.0f;

constexpr float turnSpeed = 10.0f;
constexpr float equipTime = 3.0f;

PlayerController::PlayerController(Camera& camera, EntityManager& entityManager, PhysicsWorld* physicsWorld)
	: entityManager(entityManager),
	physicsWorld(physicsWorld),
	camera(camera),
    equipped(false),
    animationStateMachine(nullptr),
    unequipIdle({ MeshManager::GetAnimation("assets/models/Unequip_Idle.fbx"), 0.0f, true }),
    unequipWalk({ MeshManager::GetAnimation("assets/models/Unequip_Walk.fbx"), 0.0f, true }),
    unequipRun({ MeshManager::GetAnimation("assets/models/Unequip_Run.fbx"), 0.0f, true }),
    jump({ MeshManager::GetAnimation("assets/models/Jump.fbx"), 0.8f, false }),
    equip({ MeshManager::GetAnimation("assets/models/Equip.fbx"), 1.0f, false, false }),
    unequip({ MeshManager::GetAnimation("assets/models/Unequip.fbx"), 1.0f, false, false }),
    equipIdle({ MeshManager::GetAnimation("assets/models/Equip_Idle.fbx"), 0.0f, true }),
    equipRunF({ MeshManager::GetAnimation("assets/models/8Way_Run_F.fbx"), 0.0f, true }),
    equipRunFR({ MeshManager::GetAnimation("assets/models/8Way_Run_FR.fbx"), 0.0f, true }),
    equipRunFL({ MeshManager::GetAnimation("assets/models/8Way_Run_FL.fbx"), 0.0f, true }),
    equipRunL({ MeshManager::GetAnimation("assets/models/8Way_Run_L.fbx"), 0.0f, true }),
    equipRunR({ MeshManager::GetAnimation("assets/models/8Way_Run_R.fbx"), 0.0f, true }),
    equipRunB({ MeshManager::GetAnimation("assets/models/8Way_Run_B.fbx"), 0.0f, true }),
    equipRunBR({ MeshManager::GetAnimation("assets/models/8Way_Run_BR.fbx"), 0.0f, true }),
    equipRunBL({ MeshManager::GetAnimation("assets/models/8Way_Run_BL.fbx"), 0.0f, true }),
    equipWalkF({ MeshManager::GetAnimation("assets/models/8Way_Walk_F.fbx"), 0.0f, true }),
    equipWalkFR({ MeshManager::GetAnimation("assets/models/8Way_Walk_FR.fbx"), 0.0f, true }),
    equipWalkFL({ MeshManager::GetAnimation("assets/models/8Way_Walk_FL.fbx"), 0.0f, true }),
    equipWalkL({ MeshManager::GetAnimation("assets/models/8Way_Walk_L.fbx"), 0.0f, true }),
    equipWalkR({ MeshManager::GetAnimation("assets/models/8Way_Walk_R.fbx"), 0.0f, true }),
    equipWalkB({ MeshManager::GetAnimation("assets/models/8Way_Walk_B.fbx"), 0.0f, true }),
    equipWalkBR({ MeshManager::GetAnimation("assets/models/8Way_Walk_BR.fbx"), 0.0f, true }),
    equipWalkBL({ MeshManager::GetAnimation("assets/models/8Way_Walk_BL.fbx"), 0.0f, true })
{

}

void PlayerController::OnAttach()
{
    AnimatedMesh* playerMesh = MeshManager::GetAnimatedMesh("assets/models/Character.FBX");

    playerEntity = entityManager.CreateEntity("Player");
    playerEntity->AddComponent<PositionComponent>(glm::vec3(10.0f, 50.0f, 0.0f));
    playerEntity->AddComponent<RotationComponent>(glm::quat(-0.7071, 0.7071f, 0.0f, 0.0f));
    playerEntity->AddComponent<ScaleComponent>(glm::vec3(0.1f));

    RenderComponent::RenderInfo testInfo;
    testInfo.mesh = playerMesh;
    testInfo.isColorOverride = true;
    testInfo.colorOverride = glm::vec3(0.0f, 0.0f, 0.6f);
    testInfo.faceCullType = FaceCullType::None;
    testInfo.roughness = 0.9f;
    playerEntity->AddComponent<RenderComponent>(testInfo);

    playerEntity->AddComponent<SkeletalAnimationComponent>();
    animComp = playerEntity->GetComponent<SkeletalAnimationComponent>();
    animComp->lerpSpeed = 5.0f;
    animationStateMachine.animComp = animComp;
    animationStateMachine.SetState(unequipIdle, animIdleSpeed);
    
    btTransform t;
    t.setOrigin(btVector3(0.0f, 100.0f, 0.0f));

    // Setup player controller ghost object
    btCapsuleShape* capsuleShape = new btCapsuleShape(1.0f, 3.0f);
    ghostObj = new btPairCachingGhostObject();
    ghostObj->setCollisionShape(capsuleShape);
    ghostObj->setCollisionFlags(btCollisionObject::CollisionFlags::CF_CHARACTER_OBJECT);
    ghostObj->setWorldTransform(t);
    physicsWorld->GetBulletWorld()->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback()); // Add default ghost object callback to physics world
    physicsWorld->GetBulletWorld()->addCollisionObject(ghostObj, btCollisionObject::CollisionFlags::CF_CHARACTER_OBJECT, btBroadphaseProxy::CollisionFilterGroups::AllFilter);

    // Setup character controller
    btController = new btKinematicCharacterController(ghostObj, capsuleShape, 5.0f, btVector3(0.0f, 1.0f, 0.0f));
    btController->setGravity(btVector3(0.0f, -100.0f, 0.0f));
    physicsWorld->GetBulletWorld()->addAction(btController); // Add controller to world
}

void PlayerController::OnUpdate(float deltaTime)
{
    glm::vec3 vel(0.0f);
    glm::vec3 cameraDir = glm::normalize(glm::vec3(camera.front.x, 0.0f, camera.front.z));

    bool animLocked = animationStateMachine.IsAnimationLocked();

    if (InputManager::GetKey(GLFW_KEY_R)->IsJustPressed() && !animLocked)
    {
        animationStateMachine.SetState(equipped ? unequip : equip, animEquipSpeed);
        equipped = !equipped;
    }

    bool movingFront = false;
    bool movingBack = false;
    bool movingSideways = false;
    if (animationStateMachine.CanMove())
    {
        if (InputManager::GetKey(GLFW_KEY_W)->IsPressed())
        {
            vel += cameraDir;
            movingFront = true;
        }
        else if (InputManager::GetKey(GLFW_KEY_S)->IsPressed())
        {
            vel += -cameraDir;
            movingBack = true;
        }

        if (InputManager::GetKey(GLFW_KEY_A)->IsPressed())
        {
            vel -= glm::normalize(glm::cross(cameraDir, glm::vec3(0.0f, 1.0f, 0.0f)));
            movingSideways = true;
        }
        else if (InputManager::GetKey(GLFW_KEY_D)->IsPressed())
        {
            vel += glm::normalize(glm::cross(cameraDir, glm::vec3(0.0f, 1.0f, 0.0f)));
            movingSideways = true;
        }
    }
    
    bool moving = vel != glm::vec3(0.0f);
    bool sprinting = moving && InputManager::GetKey(GLFW_KEY_LEFT_SHIFT)->IsPressed();

    float speedMult = 1.0f;
    if (sprinting)
    {
        if (equipped)
        {
            speedMult = equipRunSpeed;
            if (movingBack || (movingSideways && !movingFront))
            {
                speedMult *= 0.5f;
            }
        }
        else
        {
            speedMult = runSpeed;
        }
    }
    else
    {
        speedMult = walkSpeed;
    }

    if(moving) vel = glm::normalize(vel) * speedMult;

    if (InputManager::GetKey(GLFW_KEY_SPACE)->IsJustPressed() && btController->onGround() && !animLocked)
    {
        btController->jump(btVector3(0.0f, 60.0f, 0.0f));

        if(equipped) animationStateMachine.SetState(jump, animJumpSpeed);
    }
 
    if (equipped)
    {
        glm::quat rot = glm::quatLookAt(-cameraDir, glm::vec3(0.0f, 1.0f, 0.0f));
        rot = glm::rotate(rot, -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        playerEntity->GetComponent<RotationComponent>()->value = glm::slerp(playerEntity->GetComponent<RotationComponent>()->value, rot, 10.0f * deltaTime);

        if (!animLocked)
        {
            if (moving)
            {
                glm::vec3 velN = glm::normalize(vel);
                float dot = glm::dot(velN, cameraDir);
                glm::vec3 right = glm::cross(cameraDir, glm::vec3(0.0f, 1.0f, 0.0f));
                float rightDot = glm::dot(velN, right);

                float speed = sprinting ? animSprintSpeed : animWalkSpeed;

                if (glm::abs(dot - 1.0f) < 0.0001f) // Walking forward
                {
                    animationStateMachine.SetState(sprinting ? equipRunF : equipWalkF , speed);
                }
                else if (glm::abs(dot - -1.0f) < 0.0001f) // Walking back
                {
                    animationStateMachine.SetState(sprinting ? equipRunB : equipWalkB, speed);
                }
                else if (rightDot > 0.0f) // Walking right
                {
                    if (glm::abs(rightDot - 1.0f) < 0.0001f)
                    {
                        animationStateMachine.SetState(sprinting ? equipRunR : equipWalkR, speed);
                    }
                    else if(dot > 0.0f)
                    {
                        animationStateMachine.SetState(sprinting ? equipRunFR : equipWalkFR, speed);
                    }
                    else if (dot < 0.0f)
                    {
                        animationStateMachine.SetState(sprinting ? equipRunBR : equipWalkBR, speed);
                    }
                }
                else if (rightDot < 0.0f) // Walking Left
                {
                    if (glm::abs(rightDot - -1.0f) < 0.0001f)
                    {
                        animationStateMachine.SetState(sprinting ? equipRunL : equipWalkL, speed);
                    }
                    else if (dot > 0.0f)
                    {
                        animationStateMachine.SetState(sprinting ? equipRunFL : equipWalkFL, speed);
                    }
                    else if (dot < 0.0f)
                    {
                        animationStateMachine.SetState(sprinting ? equipRunBL : equipWalkBL, speed);
                    }
                }
            }
            else // Play idle
            {
                animationStateMachine.SetState(equipIdle, animIdleSpeed);
            }
        }
        
    }
    else
    {
        if (moving)
        {
            glm::quat rot = glm::quatLookAt(-glm::normalize(vel), glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            playerEntity->GetComponent<RotationComponent>()->value = glm::slerp(playerEntity->GetComponent<RotationComponent>()->value, rot, 10.0f * deltaTime);

            if (sprinting)
            {
                animationStateMachine.SetState(unequipRun, animSprintSpeed);
            }
            else
            {
                animationStateMachine.SetState(unequipWalk, animWalkSpeed);
            }
        }
        else if(!animLocked)// Play idle
        {
            animationStateMachine.SetState(unequipIdle, animIdleSpeed);
        }
    }

    btVector3 walkDirection = BulletUtils::GLMVec3ToBullet(vel);
    btController->setWalkDirection(walkDirection);
    btController->updateAction(physicsWorld->GetBulletWorld(), deltaTime);

    glm::vec3 pos = BulletUtils::BulletVec3ToGLM(ghostObj->getWorldTransform().getOrigin());
    camera.position = pos - (camera.front * 40.0f) + glm::vec3(0.0f, 10.0f, 0.0f);
    playerEntity->GetComponent<PositionComponent>()->value = pos;
    playerEntity->GetComponent<PositionComponent>()->value.y -= 1.0f;
}