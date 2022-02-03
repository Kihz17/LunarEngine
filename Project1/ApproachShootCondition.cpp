#include "ApproachShootCondition.h"
#include "GLCommon.h"

ApproachShootCondition::ApproachShootCondition(SeekBehaviour* behaviour, float shootInterval)
	: behaviour(behaviour),
	shootInterval(shootInterval),
	lastShootTime(-1.0f)
{

}

ApproachShootCondition::~ApproachShootCondition()
{

}

bool ApproachShootCondition::CanUse()
{
	return true;
}

bool ApproachShootCondition::CanContinueToUse()
{
	return true;
}

void ApproachShootCondition::OnStart()
{

}

void ApproachShootCondition::OnStop()
{

}

void ApproachShootCondition::Update(float deltaTime)
{
	if (!behaviour->IsWithinRadius()) return; // Not within radius, don't even try to shoot

	float currentTime = (float) glfwGetTime();
	if (lastShootTime == -1.0f || (currentTime - lastShootTime) >= shootInterval) // We can shoot!
	{
		// TODO: Shoot the thing
	}
}