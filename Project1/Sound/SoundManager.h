#pragma once

#include "DSP.h"

#include <glm/glm.hpp>

#include<unordered_map>
#include <iostream>
#include <string>

struct ListenerInfo
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 direction;
	glm::vec3 up;
};

struct PlaySoundInfo
{
	bool startPaused = true;
	float volume = 0.5f;
	std::vector<DSPObject*> dsps;
};

class SoundManager
{
public:
	static void Initilaize();
	static void CleanUp(bool releaseSystem = true);

	static void Update(float deltaTime, const ListenerInfo& info);

	static FMOD::Sound* CreateSound3D(const std::string& name, const std::string& path, const glm::vec3& position);
	static DSPObject* CreateDSP(FMOD_DSP_TYPE dspType);

	static FMOD::Channel* PlaySound(const std::string& name, const PlaySoundInfo& info);
	static FMOD::Channel* PlaySound(FMOD::Sound* sound, const PlaySoundInfo& info);

	static FMOD::System* GetSystem() { return system; }

private:
	static FMOD::System* system;

	static std::unordered_map<std::string, FMOD::Sound*> sounds;

	static FMOD_RESULT result;
};