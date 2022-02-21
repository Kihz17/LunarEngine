#include "FormationLayer.h"
#include "UUID.h"
#include "Components.h"
#include "FormationCondition.h"
#include "Utils.h"
#include "InputManager.h"
#include "FlockingCondition.h"

#include<Shapes.h>

const std::vector <glm::vec3> squareFormation =
{
    glm::vec3(10.0f, 0.0f, -10.0f),
    glm::vec3(10.0f, 0.0f, -4.0f),
    glm::vec3(10.0f, 0.0f, 4.0f),
    glm::vec3(10.0f, 0.0f, 10.0f),

    glm::vec3(4.0f, 0.0f, 10.0f),
    glm::vec3(-4.0f, 0.0f, 10.0f),
    glm::vec3(-10.0f, 0.0f, 10.0f),

    glm::vec3(-10.0f, 0.0f, 4.0f),
    glm::vec3(-10.0f, 0.0f, -4.0f),
    glm::vec3(-10.0f, 0.0f, -10.0f),

    glm::vec3(-4.0f, 0.0f, -10.0f),
    glm::vec3(4.0f, 0.0f, -10.0f),
};

const std::vector <glm::vec3> circleFormation =
{
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(5.0f, 0.0f, -8.0f),
    glm::vec3(8.0f, 0.0f, -5.0f),

    glm::vec3(10.0f, 0.0f, 0.0f),
    glm::vec3(8.0f, 0.0f, 5.0f),
    glm::vec3(5.0f, 0.0f, 8.0f),

    glm::vec3(0.0f, 0.0f, 10.0f),
    glm::vec3(-5.0f, 0.0f, 8.0f),
    glm::vec3(-8.0f, 0.0f, 5.0f),
 
    glm::vec3(-10.0f, 0.0f, 0.0f),
    glm::vec3(-8.0f, 0.0f, -5.0f),
    glm::vec3(-5.0f, 0.0f, -8.0f),
};

const std::vector <glm::vec3> vFormation =
{
    glm::vec3(0.0f, 0.0f, -10.0f),

    glm::vec3(3.0f, 0.0f, -5.0f),
    glm::vec3(-3.0f, 0.0f, -5.0f),

    glm::vec3(4.5f, 0.0f, 0.0f),
    glm::vec3(-4.5f, 0.0f, 0.0f),

    glm::vec3(6.0f, 0.0f, 5.0f),
    glm::vec3(-6.0f, 0.0f, 5.0f),

    glm::vec3(0.0f, 0.0f, 0.0f),

    glm::vec3(-7.5f, 0.0f, 10.0f),
    glm::vec3(7.5f, 0.0f, 10.0f),

    glm::vec3(9.0f, 0.0f, 15.0f),
    glm::vec3(-9.0f, 0.0f, 15.0f),
};

const std::vector <glm::vec3> lineFormation =
{
    glm::vec3(30.0f, 0.0f, 0.0f),
    glm::vec3(25.0f, 0.0f, 0.0f),
    glm::vec3(20.0f, 0.0f, 0.0f),
    glm::vec3(15.0f, 0.0f, 0.0f),
    glm::vec3(10.0f, 0.0f, 0.0f),
    glm::vec3(5.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(-5.0f, 0.0f, 0.0f),
    glm::vec3(-10.0f, 0.0f, 0.0f),
    glm::vec3(-15.0f, 0.0f, 0.0f),
    glm::vec3(-20.0f, 0.0f, 0.0f),
    glm::vec3(-25.0f, 0.0f, 0.0f),
};

const std::vector <glm::vec3> rowFormation =
{
    glm::vec3(15.0f, 0.0f, 0.0f),
    glm::vec3(10.0f, 0.0f, 0.0f),
    glm::vec3(5.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(-5.0f, 0.0f, 0.0f),
    glm::vec3(-10.0f, 0.0f, 0.0f),

    glm::vec3(15.0f, 0.0f, 10.0f),
    glm::vec3(10.0f, 0.0f, 10.0f),
    glm::vec3(5.0f, 0.0f, 10.0f),
    glm::vec3(0.0f, 0.0f, 10.0f),
    glm::vec3(-5.0f, 0.0f, 10.0f),
    glm::vec3(-10.0f, 0.0f, 10.0f),
};

std::vector<Entity*> testEntities;

FormationLayer::FormationLayer(const FormationLayerInfo& info)
	: entityMesh(info.mesh),
    pathMesh(info.pathMeshes),
	entityManager(info.entityManager),
	physicsFactory(info.physicsFactory),
	physicsWorld(info.physicsWorld),
    flockingGroup(new FlockingGroup()),
    followPath(false),
    pathPos(0.0f)
{

}

FormationLayer::~FormationLayer()
{
    delete flockingGroup;
}

void FormationLayer::OnAttach()
{
    coordinator.SetPositionOffsets(circleFormation);

    path.nodes.push_back(glm::vec3(-40.0f, 0.0f, -40.0f));
    path.nodes.push_back(glm::vec3(40.0f, 0.0f, -40.0f));
    path.nodes.push_back(glm::vec3(-40.0f, 0.0f, 40.0f));
    path.nodes.push_back(glm::vec3(40.0f, 0.0f, 40.0f));
    path.nodes.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    path.currentNode = 0;
    path.arriveRadius = 2.0f;
    path.pathRadius = 1.0f;
    path.pathDirection = 1;
    path.repeating = false;

    for (glm::vec3& pos : path.nodes)
    {
        Entity* entity = entityManager->CreateEntity(std::to_string(UUID()));
        entity->AddComponent<PositionComponent>(pos);
        entity->AddComponent<ScaleComponent>();
        entity->AddComponent<RotationComponent>();

        RenderComponent::RenderInfo renderInfo;
        renderInfo.mesh = pathMesh;
        renderInfo.isColorOverride = true;
        renderInfo.colorOverride = glm::vec3(0.8f, 0.0f, 0.0f);
        entity->AddComponent<RenderComponent>(renderInfo);
        pathNodeEntities.push_back(entity);
    }

    for (int i = 0; i < 6; i++)
    {
        testEntities.push_back(SpawnTestEntity(glm::vec3(i * 8.0f, 1.0f, 0.0f)));
    }

    for (int i = 0; i < 6; i++)
    {
        testEntities.push_back(SpawnTestEntity(glm::vec3(i * 8.0f, 1.0f, 10.0f)));
    }
}

void FormationLayer::OnUpdate(float deltaTime)
{
    if (InputManager::GetKey(GLFW_KEY_1)->IsJustPressed())
    {
        SetFlocking(false);
        coordinator.SetPositionOffsets(circleFormation);
    }
    else if (InputManager::GetKey(GLFW_KEY_2)->IsJustPressed())
    {
        SetFlocking(false);
        coordinator.SetPositionOffsets(vFormation);
    }
    else if (InputManager::GetKey(GLFW_KEY_3)->IsJustPressed())
    {
        SetFlocking(false);
        coordinator.SetPositionOffsets(squareFormation);
    }
    else if (InputManager::GetKey(GLFW_KEY_4)->IsJustPressed())
    {
        SetFlocking(false);
        coordinator.SetPositionOffsets(lineFormation);
    }
    else if (InputManager::GetKey(GLFW_KEY_5)->IsJustPressed())
    {
        SetFlocking(false);
        coordinator.SetPositionOffsets(rowFormation);
    }
    else if (InputManager::GetKey(GLFW_KEY_6)->IsJustPressed())
    {
        SetFlocking(true);
    }
    else if (InputManager::GetKey(GLFW_KEY_7)->IsJustPressed())
    {
        SetFlocking(false);
    }

    if (InputManager::GetKey(GLFW_KEY_Z)->IsJustPressed()) // Decrease cohestion
    {
        ChangeFlockingWeight(-0.1f, FlockingWeightType::Cohesion);
    } 
    else if (InputManager::GetKey(GLFW_KEY_X)->IsJustPressed()) // Increase cohestion
    {
        ChangeFlockingWeight(0.1f, FlockingWeightType::Cohesion);
    }

    if (InputManager::GetKey(GLFW_KEY_C)->IsJustPressed()) // Decrease separation
    {
        ChangeFlockingWeight(-0.1f, FlockingWeightType::Separation);
    }
    else if (InputManager::GetKey(GLFW_KEY_V)->IsJustPressed()) // Increase separation
    {
        ChangeFlockingWeight(0.1f, FlockingWeightType::Separation);
    }

    if (InputManager::GetKey(GLFW_KEY_B)->IsJustPressed()) // Decrease alignment
    {
        ChangeFlockingWeight(-0.1f, FlockingWeightType::Alignment);
    }
    else if (InputManager::GetKey(GLFW_KEY_N)->IsJustPressed()) // Increase alignment
    {
        ChangeFlockingWeight(0.1f, FlockingWeightType::Alignment);
    }

    if (InputManager::GetKey(GLFW_KEY_8)->IsJustPressed())
    {
        followPath = true;
    }
    else if (InputManager::GetKey(GLFW_KEY_0)->IsJustPressed())
    {
        followPath = false;
    }

    if (InputManager::GetKey(GLFW_KEY_9)->IsJustPressed()) // Decrease alignment
    {
        path.pathDirection = path.pathDirection > 0 ? -1 : 1;
        path.currentNode = std::max(0, std::min(path.currentNode + path.pathDirection, (int) path.nodes.size()));
    }

    if (followPath)
    {
        // Try to move to the next path node
        glm::vec3& targetPos = path.nodes[path.currentNode];
        if (path.nodes.size() > 1)
        {
            float distance = glm::length(pathPos - targetPos);
            if (distance < path.pathRadius) // We have arrived, move to next
            {
                int indexBefore = path.currentNode;
                path.currentNode = std::max(0, std::min(path.pathDirection + path.currentNode, (int) (path.nodes.size() - 1)));
                if (path.repeating) // Check if we should start back at the beginning
                {
                    if (path.pathDirection > 0 && path.currentNode >= path.nodes.size())
                    {
                        path.currentNode = path.currentNode - path.nodes.size();
                    }
                    else if (path.pathDirection < 0 && path.currentNode < 0)
                    {
                        path.currentNode = path.nodes.size() + path.currentNode;
                    }
                }

                Entity* pathBefore = pathNodeEntities[indexBefore];
                pathBefore->GetComponent<RenderComponent>()->colorOverride = glm::vec3(0.8f, 0.0f, 0.0f);
            }

            Entity* newPath = pathNodeEntities[path.currentNode];
            newPath->GetComponent<RenderComponent>()->colorOverride = glm::vec3(0.0f, 0.8f, 0.0f);
        }

        // Update path position
        {
            glm::vec3 direction = targetPos - pathPos;
            glm::vec3 velocity = glm::normalize(direction) * 5.0f;
            float distance = glm::length(direction);
            if (distance < path.arriveRadius) // Slow down on arrival
            {
                velocity *= ((distance - 0.75f) / path.arriveRadius);
            }

            if (glm::length(velocity) > 50.0f)
            {
                velocity = glm::normalize(velocity) * 50.0f;
            }

            pathPos += velocity * deltaTime;
        }
      

        UpdateFlockingDirection(pathPos);
        coordinator.SetCentralPosition(pathPos);
    }

    coordinator.Update(deltaTime);
}

Entity* FormationLayer::SpawnTestEntity(const glm::vec3& position)
{
    Entity* testEntity = entityManager->CreateEntity(std::to_string(UUID()));
    testEntity->AddComponent<PositionComponent>();
    testEntity->AddComponent<RotationComponent>();
    testEntity->AddComponent<ScaleComponent>(glm::vec3(0.01f, 0.01f, 0.01f));

    Physics::RigidBodyInfo rigidInfo;
    rigidInfo.linearDamping = 0.99f;
    rigidInfo.angularDamping = 0.00f;
    rigidInfo.isStatic = false;
    rigidInfo.mass = 1.0f;
    rigidInfo.position = position;
    rigidInfo.linearVelocity = glm::vec3(0.0f);
    rigidInfo.friction = 0.95f;
    RigidBodyComponent* rigidComp = testEntity->AddComponent<RigidBodyComponent>(physicsFactory->CreateRigidBody(rigidInfo, new Physics::SphereShape(0.5f)));
    physicsWorld->AddRigidBody(rigidComp->ptr, testEntity);

    SteeringBehaviourComponent* steeringComp = testEntity->AddComponent<SteeringBehaviourComponent>();
    FormationBehaviour* formation = new FormationBehaviour(rigidComp->ptr, 30.0f, 1.0f, 50.0f);
    steeringComp->AddBehaviour(0, new FormationCondition(formation));

    FlockingInfo flockingInfo;
    flockingInfo.group = flockingGroup;
    flockingInfo.separationRadius = 4.0f;
    flockingInfo.alignmentRadius = 2.0f;
    flockingInfo.cohesionRadius = 30.0f;
    flockingInfo.separationWeight = 0.4f;
    flockingInfo.alignmentWeight = 0.1f;
    flockingInfo.cohesionWeight = 0.5f;
    steeringComp->AddTargetingBehaviour(0, new FlockingCondition(new FlockingBehaviour(rigidComp->ptr, flockingInfo, 30.0f, 1.0f, 50.0f)));

    RenderComponent::RenderInfo mehsInfo;
    mehsInfo.mesh = entityMesh;
    mehsInfo.isColorOverride = true;
    mehsInfo.colorOverride = glm::vec3(0.0f, 0.0f, 0.9f);
    testEntity->AddComponent<RenderComponent>(mehsInfo);

    coordinator.AddFormationEntity(formation);
    flockingGroup->rigidbodies.push_back(rigidComp);

    return testEntity;
}

void FormationLayer::SetFlocking(bool b)
{
    for (Entity* e : testEntities)
    {
        SteeringBehaviourComponent* steeringComp = e->GetComponent<SteeringBehaviourComponent>();
        FlockingCondition* c = dynamic_cast<FlockingCondition*>(steeringComp->GetBehaviour(0, SteeringBehaviourType::Targeting));
        c->canUse = b;
    }
}

void FormationLayer::ChangeFlockingWeight(float modification, FlockingWeightType type)
{
    float cWeight = 0.0f;
    float sWeight = 0.0f;
    float aWeight = 0.0f;

    for (Entity* e : testEntities)
    {
        SteeringBehaviourComponent* steeringComp = e->GetComponent<SteeringBehaviourComponent>();
        FlockingCondition* c = dynamic_cast<FlockingCondition*>(steeringComp->GetBehaviour(0, SteeringBehaviourType::Targeting));
        FlockingBehaviour* fb = dynamic_cast<FlockingBehaviour*>(c->GetBehaviour());

        if (type == FlockingWeightType::Cohesion)
        {
            fb->cohesionWeight += modification;
            fb->alignmentWeight -= modification * 0.5f;
            fb->separationWeight -= modification * 0.5f;
        }
        else if (type == FlockingWeightType::Alignment)
        {
            fb->alignmentWeight += modification;
            fb->cohesionWeight -= modification * 0.5f;
            fb->separationWeight -= modification * 0.5f;
        }
        else if (type == FlockingWeightType::Separation)
        {
            fb->separationWeight += modification;
            fb->alignmentWeight -= modification * 0.5f;
            fb->cohesionWeight -= modification * 0.5f;
        }

        cWeight = fb->cohesionWeight;
        sWeight = fb->separationWeight;
        aWeight = fb->alignmentWeight;
    }

    std::cout << "Cohesion Weight: " << cWeight << "\n";
    std::cout << "Separation Weight: " << sWeight << "\n";
    std::cout << "Alignment Weight: " << aWeight << "\n\n";
}

void FormationLayer::UpdateFlockingDirection(const glm::vec3& pos)
{
    for (Entity* e : testEntities)
    {
        SteeringBehaviourComponent* steeringComp = e->GetComponent<SteeringBehaviourComponent>();
        FlockingCondition* c = dynamic_cast<FlockingCondition*>(steeringComp->GetBehaviour(0, SteeringBehaviourType::Targeting));
        FlockingBehaviour* fb = dynamic_cast<FlockingBehaviour*>(c->GetBehaviour());
        fb->moveDirection = glm::normalize(pos - fb->GetRigidBody()->GetPosition()) * 15.0f;
    }
}