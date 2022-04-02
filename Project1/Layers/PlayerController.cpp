#include "PlayerController.h"
#include "Components.h"
#include "InputManager.h"
#include "MeshManager.h"
#include "Utils.h"
#include "TextureManager.h"
#include "Texture2D.h"

#include <glm/gtx/vector_angle.hpp>
#include <iostream>

constexpr float velSpeed = 10.0f;

constexpr float walkSpeed = 0.1f;
constexpr float runSpeed = 0.3f;
constexpr float equipRunSpeed = 0.2f;

constexpr float animIdleSpeed = 15.0f;
constexpr float animWalkSpeed = 15.0f;
constexpr float animSprintSpeed = 17.0f;
constexpr float animEquipSpeed = 19.0f;
constexpr float animJumpSpeed = 2.0f;

constexpr float turnSpeed = 10.0f;
constexpr float equipTime = 3.0f;

constexpr float animAttackSpeed = 17.0f;

PlayerController::PlayerController(Camera& camera, EntityManager& entityManager, PhysicsWorld* physicsWorld)
    : entityManager(entityManager),
    physicsWorld(physicsWorld),
    camera(camera),
    equipped(false),
    animationStateMachine(nullptr),
    unequipIdle({ MeshManager::GetAnimation("assets/models/Unequip_Idle.fbx"), 0.0f, true, 15.0f }),
    unequipWalk({ MeshManager::GetAnimation("assets/models/Unequip_Walk.fbx"), 0.0f, true, 15.0f }),
    unequipRun({ MeshManager::GetAnimation("assets/models/Unequip_Run.fbx"), 0.0f, true, 17.0f }),
    jump({ MeshManager::GetAnimation("assets/models/Jump.fbx"), 0.8f, false, 20.0f }),
    equip({ MeshManager::GetAnimation("assets/models/Equip.fbx"), 1.0f, false, 19.0f, 3 }),
    unequip({ MeshManager::GetAnimation("assets/models/Unequip.fbx"), 1.0f, false, 19.0f, 3 }),
    equipIdle({ MeshManager::GetAnimation("assets/models/Equip_Idle.fbx"), 0.0f, true, 15.0f }),
    equipRunF({ MeshManager::GetAnimation("assets/models/8Way_Run_F.fbx"), 0.0f, true, 15.0f }),
    equipRunFR({ MeshManager::GetAnimation("assets/models/8Way_Run_FR.fbx"), 0.0f, true, 15.0f }),
    equipRunFL({ MeshManager::GetAnimation("assets/models/8Way_Run_FL.fbx"), 0.0f, true, 15.0f }),
    equipRunL({ MeshManager::GetAnimation("assets/models/8Way_Run_L.fbx"), 0.0f, true, 15.0f }),
    equipRunR({ MeshManager::GetAnimation("assets/models/8Way_Run_R.fbx"), 0.0f, true, 15.0f }),
    equipRunB({ MeshManager::GetAnimation("assets/models/8Way_Run_B.fbx"), 0.0f, true, 15.0f }),
    equipRunBR({ MeshManager::GetAnimation("assets/models/8Way_Run_BR.fbx"), 0.0f, true, 15.0f }),
    equipRunBL({ MeshManager::GetAnimation("assets/models/8Way_Run_BL.fbx"), 0.0f, true, 15.0f }),
    equipWalkF({ MeshManager::GetAnimation("assets/models/8Way_Walk_F.fbx"), 0.0f, true, 15.0f }),
    equipWalkFR({ MeshManager::GetAnimation("assets/models/8Way_Walk_FR.fbx"), 0.0f, true, 15.0f }),
    equipWalkFL({ MeshManager::GetAnimation("assets/models/8Way_Walk_FL.fbx"), 0.0f, true, 15.0f }),
    equipWalkL({ MeshManager::GetAnimation("assets/models/8Way_Walk_L.fbx"), 0.0f, true, 15.0f }),
    equipWalkR({ MeshManager::GetAnimation("assets/models/8Way_Walk_R.fbx"), 0.0f, true, 15.0f }),
    equipWalkB({ MeshManager::GetAnimation("assets/models/8Way_Walk_B.fbx"), 0.0f, true, 15.0f }),
    equipWalkBR({ MeshManager::GetAnimation("assets/models/8Way_Walk_BR.fbx"), 0.0f, true, 15.0f }),
    equipWalkBL({ MeshManager::GetAnimation("assets/models/8Way_Walk_BL.fbx"), 0.0f, true, 15.0f }),
    attack1({ MeshManager::GetAnimation("assets/models/Combo01_1.fbx"), 0.7f, true, 20.0f, 1 }),
    attack2({ MeshManager::GetAnimation("assets/models/Combo01_2.fbx"), 0.7f, true, 23.0f, 1 }),
    attack3({ MeshManager::GetAnimation("assets/models/Combo01_3.fbx"), 0.8f, true, 23.0f, 1 }),
    attack4({ MeshManager::GetAnimation("assets/models/Combo01_4.fbx"), 0.9f, true, 19.0f, 1 })
{
    basicAttack.attackStages.push_back({ attack1, 0.2f, 10.0f });
    basicAttack.attackStages.push_back({ attack2, 0.25f, 10.0f });
    basicAttack.attackStages.push_back({ attack3, 0.25f, 10.0f });
    basicAttack.attackStages.push_back({ attack4, 0.1, 10.0f });
}

void PlayerController::OnAttach()
{
    AnimatedMesh* playerMesh = MeshManager::GetAnimatedMesh("assets/models/Character.FBX");

    playerEntity = entityManager.CreateEntity("Player");
    playerEntity->shouldSave = false;
    playerEntity->AddComponent<PositionComponent>(glm::vec3(10.0f, 5.0f, 0.0f));
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
    animationStateMachine.SetState(unequipIdle);
    
    btTransform t;
    t.setOrigin(btVector3(0.0f, 10.0f, 0.0f));

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

    // Setup lantern hinge
    Physics::RigidBodyInfo lanternRigidInfo;
    lanternRigidInfo.mass = 0.0f;
    lanternRigidInfo.position = glm::vec3(0.0f, 5.0f, 0.0f);
    lanternHinge = new RigidBody(lanternRigidInfo, new Physics::SphereShape(0.1f));

    btRigidBody* btLanternAnchor = lanternHinge->GetBulletBody();
    btLanternAnchor->setCollisionFlags(btLanternAnchor->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    btLanternAnchor->setActivationState(DISABLE_DEACTIVATION);
    physicsWorld->AddBody(lanternHinge);

    {
        Entity* lanternE = entityManager.CreateEntity();
        lanternE->shouldSave = false;

        lanternE->AddComponent<PositionComponent>();
        lanternE->AddComponent<RotationComponent>();
        lanternE->AddComponent<ScaleComponent>(glm::vec3(0.04f));

        RenderComponent::RenderInfo lanternInfo;
        lanternInfo.mesh = MeshManager::GetMesh("assets/models/Lantern.fbx");
        lanternInfo.albedoTextures.push_back({ TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Doors_BC.TGA", TextureFilterType::Linear, TextureWrapType::Repeat), 1.0f });
        lanternInfo.normalTexture = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Doors_N.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
        lanternInfo.ormTexture = TextureManager::CreateTexture2D("assets/textures/FantasyVillage/T_Doors_ORM.TGA", TextureFilterType::Linear, TextureWrapType::Repeat);
        lanternInfo.colorOverride = glm::vec3(0.0f, 1.0f, 0.0f);
        lanternE->AddComponent<RenderComponent>(lanternInfo);

        Physics::RigidBodyInfo lanternRigidInfo;
        lanternRigidInfo.mass = 1.0f;
        lanternRigidInfo.position = glm::vec3(0.0f, 0.0f, 0.0f);
        lanternRigidBody = new RigidBody(lanternRigidInfo, new Physics::BoxShape(glm::vec3(0.2f, 0.4f, 0.2f)));
        lanternRigidBody->GetBulletBody()->setDamping(0.8f, 0.9f);
        lanternE->AddComponent<RigidBodyComponent>(lanternRigidBody);

        physicsWorld->AddBody(lanternRigidBody);

        btTypedConstraint* constraint = new btPoint2PointConstraint(*lanternRigidBody->GetBulletBody(), *btLanternAnchor, btVector3(0.0f, -0.5f, 0.6f), btVector3(0.0f, 0.0f, 0.0f));
        physicsWorld->GetBulletWorld()->addConstraint(constraint);
    }

    LightInfo lightInfo;
    lightInfo.color = glm::vec3(0.83f, 0.7f, 0.1f);
    lightInfo.lightType = LightType::Point;
    lightInfo.on = false;
    lightInfo.intensity = 100.0f;
    lightInfo.radius = 300.0f;
    lanternLight = new Light(lightInfo);
}

void PlayerController::OnUpdate(float deltaTime)
{
    glm::vec3 vel(0.0f);
    glm::vec3 cameraDir = glm::normalize(glm::vec3(camera.front.x, 0.0f, camera.front.z));

    if (InputManager::GetKey(GLFW_KEY_R)->IsJustPressed() && animationStateMachine.CanPlayAnimation(equip))
    {
        animationStateMachine.SetState(equipped ? unequip : equip);
        equipped = !equipped;
    }

    bool movingFront = false;
    bool movingBack = false;
    bool movingSideways = false;
    if (animationStateMachine.CanPlayAnimation(unequipWalk) || animationStateMachine.GetAnimation() == jump.anim)
    {
        if (InputManager::GetKey(GLFW_KEY_W)->IsPressed())
        {
            vel += cameraDir * velSpeed * deltaTime;
            movingFront = true;
        }
        else if (InputManager::GetKey(GLFW_KEY_S)->IsPressed())
        {
            vel += -cameraDir * velSpeed * deltaTime;
            movingBack = true;
        }

        if (InputManager::GetKey(GLFW_KEY_A)->IsPressed())
        {
            vel -= glm::normalize(glm::cross(cameraDir, glm::vec3(0.0f, 1.0f, 0.0f))) * velSpeed * deltaTime;
            movingSideways = true;
        }
        else if (InputManager::GetKey(GLFW_KEY_D)->IsPressed())
        {
            vel += glm::normalize(glm::cross(cameraDir, glm::vec3(0.0f, 1.0f, 0.0f))) * velSpeed * deltaTime;
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

    if (InputManager::GetKey(GLFW_KEY_SPACE)->IsJustPressed() && btController->onGround())
    {
        btController->jump(btVector3(0.0f, 60.0f, 0.0f));

        if(animationStateMachine.CanPlayAnimation(jump) && equipped) animationStateMachine.SetState(jump);
    }    

    if (InputManager::GetKey(GLFW_KEY_L)->IsJustPressed())
    {
        lanternLight->UpdateOn(!lanternLight->on);
    }

    // Attack
    if (InputManager::GetKey(GLFW_MOUSE_BUTTON_1)->IsJustPressed() && equipped && animationStateMachine.CanPlayAnimation(attack1))
    {
        lastLeftClickTime = glfwGetTime();

        AttackStage& attackStage = basicAttack.attackStages[basicAttack.currentStage];
        if (attackStage.animationState.anim == animationStateMachine.GetAnimation()) // An attack animation is currently playing
        {
            float timePlayed = animationStateMachine.GetTimePlayed();   
            float animDuration = attackStage.animationState.duration;

            float minWindowTime = animDuration - attackStage.chainingWindow;
            if (timePlayed >= minWindowTime && timePlayed < animDuration) // We chanined the attack, move to the next stage
            {
                basicAttack.currentStage++;
                if (basicAttack.currentStage >= basicAttack.attackStages.size()) basicAttack.currentStage = 0; // Reset to beginning

                AttackStage& newAttackStage = basicAttack.attackStages[basicAttack.currentStage];
                animationStateMachine.SetState(newAttackStage.animationState); // Play the animation
            }
        }
        else // No attack animation is playing, we can safely assume that this is the first "Stage" of the attack
        {
            basicAttack.currentStage = 0;
            AttackStage& resetAttackStage = basicAttack.attackStages[basicAttack.currentStage];

            animationStateMachine.SetState(resetAttackStage.animationState);
        }
    }

    if (equipped)
    {
        glm::quat rot = glm::quatLookAt(-cameraDir, glm::vec3(0.0f, 1.0f, 0.0f));
        rot = glm::rotate(rot, -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        playerEntity->GetComponent<RotationComponent>()->value = glm::slerp(playerEntity->GetComponent<RotationComponent>()->value, rot, 10.0f * deltaTime);
      
        if (moving)
        {
            glm::vec3 velN = glm::normalize(vel);
            float dot = glm::dot(velN, cameraDir);
            glm::vec3 right = glm::cross(cameraDir, glm::vec3(0.0f, 1.0f, 0.0f));
            float rightDot = glm::dot(velN, right);

            if (glm::abs(dot - 1.0f) < 0.0001f) // Walking forward
            {
                animationStateMachine.SetState(sprinting ? equipRunF : equipWalkF);
            }
            else if (glm::abs(dot - -1.0f) < 0.0001f) // Walking back
            {
                animationStateMachine.SetState(sprinting ? equipRunB : equipWalkB);
            }
            else if (rightDot > 0.0f) // Walking right
            {
                if (glm::abs(rightDot - 1.0f) < 0.0001f)
                {
                    animationStateMachine.SetState(sprinting ? equipRunR : equipWalkR);
                }
                else if(dot > 0.0f)
                {
                    animationStateMachine.SetState(sprinting ? equipRunFR : equipWalkFR);
                }
                else if (dot < 0.0f)
                {
                    animationStateMachine.SetState(sprinting ? equipRunBR : equipWalkBR);
                }
            }
            else if (rightDot < 0.0f) // Walking Left
            {
                if (glm::abs(rightDot - -1.0f) < 0.0001f)
                {
                    animationStateMachine.SetState(sprinting ? equipRunL : equipWalkL);
                }
                else if (dot > 0.0f)
                {
                    animationStateMachine.SetState(sprinting ? equipRunFL : equipWalkFL);
                }
                else if (dot < 0.0f)
                {
                    animationStateMachine.SetState(sprinting ? equipRunBL : equipWalkBL);
                }
            }
        }
        else if (animationStateMachine.CanPlayAnimation(equipIdle)) // Play idle
        {
            animationStateMachine.SetState(equipIdle);
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
                animationStateMachine.SetState(unequipRun);
            }
            else
            {
                animationStateMachine.SetState(unequipWalk);
            }
        }
        else if(animationStateMachine.CanPlayAnimation(unequipIdle)) // Play idle
        {
            animationStateMachine.SetState(unequipIdle);
        }
    }

    btVector3 walkDirection = BulletUtils::GLMVec3ToBullet(vel);
    btController->setWalkDirection(walkDirection);
    btController->updateAction(physicsWorld->GetBulletWorld(), deltaTime);

    glm::vec3 pos = BulletUtils::BulletVec3ToGLM(ghostObj->getWorldTransform().getOrigin());

    // Move lantern anchor to right side of character
    btTransform t;
    t.setRotation(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    glm::quat lanternRot = playerEntity->GetComponent<RotationComponent>()->value;
    lanternRot.x = 0.0f;
    lanternRot.z = 0.0f;
    lanternRot = glm::normalize(lanternRot);
    glm::vec3 playerDir = glm::normalize(lanternRot * Utils::FrontVec());

    glm::vec3 lanternAnchorPos = pos + glm::vec3(0.0f, 10.0f, 0.0f);
    lanternAnchorPos += playerDir * 0.5f;
    lanternAnchorPos -= glm::cross(playerDir, glm::vec3(0.0f, 1.0f, 0.0f)) * 1.5f;

    t.setOrigin(BulletUtils::GLMVec3ToBullet(lanternAnchorPos));
    lanternHinge->GetBulletBody()->setWorldTransform(t);

    // Update lantern light pos
    glm::vec3 lanternPos;
    lanternRigidBody->GetPosition(lanternPos);
    lanternLight->UpdatePosition(lanternPos);

    // Align camera with player
    camera.position = pos - (camera.front * 40.0f) + glm::vec3(0.0f, 10.0f, 0.0f);
    playerEntity->GetComponent<PositionComponent>()->value = pos;
    playerEntity->GetComponent<PositionComponent>()->value.y -= 1.0f;

    // Camera collision with objects
    btVector3 rayFrom = BulletUtils::GLMVec3ToBullet(camera.position);
    btVector3 rayTo = BulletUtils::GLMVec3ToBullet(camera.position + (camera.front * glm::distance(camera.position, pos)));
    btCollisionWorld::AllHitsRayResultCallback rayResult(rayFrom, rayTo);
    physicsWorld->GetBulletWorld()->rayTest(rayFrom, rayTo, rayResult);

    if (rayResult.hasHit())
    {
        float closest = std::numeric_limits<float>::max();
        glm::vec3 closestPoint;
        for (int i = 0; i < rayResult.m_hitPointWorld.size(); i++)
        {
            glm::vec3 point = BulletUtils::BulletVec3ToGLM(rayResult.m_hitPointWorld[i]);
            float distance = glm::distance2(point, pos);
            if (distance < closest)
            {
                closest = distance;
                closestPoint = point;
            }
        }

        camera.position = closestPoint + camera.front;
    }
  
}