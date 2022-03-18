#include "SoundManager.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

FMOD::System* SoundManager::system = nullptr;
std::unordered_map<std::string, FMOD::Sound*> SoundManager::sounds;

FMOD_RESULT SoundManager::result = FMOD_OK;

void SoundManager::Initilaize()
{
	if (SoundManager::system) // Already initialized
	{
		return;
	}

	result = FMOD::System_Create(&SoundManager::system);
	if (result != FMOD_OK)
	{
		std::cout << "Failed to create system! (FMOD)" << std::endl;
		return;
	}

	result = SoundManager::system->init(32, FMOD_INIT_NORMAL, NULL);
	if (result != FMOD_OK)
	{
		std::cout << "Failed to initialize system! (FMOD)" << std::endl;
		return;
	}

	result = SoundManager::system->setStreamBufferSize(64 * 1024, FMOD_TIMEUNIT_RAWBYTES);
	if (result != FMOD_OK)
	{
		std::cout << "Unable to set file buffer size! (FMOD)" << std::endl;
		return;
	}
}

void SoundManager::CleanUp(bool releaseSystem)
{

	// Release sounds
	for (const std::pair<std::string, FMOD::Sound*>& entry : SoundManager::sounds)
	{
		entry.second->release();
	}

	SoundManager::sounds.clear();

	if (releaseSystem)
	{
		if (SoundManager::system)
		{
			result = SoundManager::system->close();
			if (result != FMOD_OK)
			{
				std::cout << "Failed to close system! (FMOD)" << std::endl;
			}

			result = SoundManager::system->release();
			if (result != FMOD_OK)
			{
				std::cout << "Failed to release system! (FMOD)" << std::endl;
			}
		}
	}
}

void SoundManager::Update(float deltaTime, const ListenerInfo& info)
{
	// Update system
	result = SoundManager::system->update();
	if (result != FMOD_OK)
	{
		std::cout << "Failed to update system! (FMOD)" << std::endl;
	}

	// Update listener pos
	FMOD_VECTOR pos;
	pos.x = info.position.x;
	pos.y = info.position.y;
	pos.z = info.position.z;

	FMOD_VECTOR vel = { info.velocity.x, info.velocity.y, info.velocity.z };
	FMOD_VECTOR up = { info.up.x, info.up.y, info.up.z };
	FMOD_VECTOR direction = { info.direction.x, info.direction.y, info.direction.z };

	result = system->set3DListenerAttributes(0, &pos, &vel, &up, &direction);
	if (result != FMOD_OK)
	{
		std::cout << "Failed to update listener attributes! (FMOD)" << std::endl;
		return;
	}
}

DSPObject* SoundManager::CreateDSP(FMOD_DSP_TYPE dspType)
{
	FMOD::DSP* dsp = nullptr;
	result = SoundManager::system->createDSPByType(dspType, &dsp);
	if (result != FMOD_OK)
	{
		std::cout << "Failed to create DSP! (FMOD)" << std::endl;
		return nullptr;
	}

	int parameterCount; // Get the amount of params this dsp has since they vary
	result = dsp->getNumParameters(&parameterCount);
	if (result != FMOD_OK)
	{
		std::cout << "Failed to DSP parameter count! (FMOD)" << std::endl;
		return nullptr;
	}

	DSPObject* dspObject = new DSPObject();

	for (int i = 0; i < parameterCount; i++) // Get param information
	{
		FMOD_DSP_PARAMETER_DESC* parameterDesc = nullptr;
		result = dsp->getParameterInfo(i, &parameterDesc);
		if (result != FMOD_OK)
		{
			std::cout << "Failed to get DSP parameter info! (FMOD)" << std::endl;
			delete dspObject;
			return nullptr;
		}

		DSPParameter param(parameterDesc);
		dspObject->parameterDescriptions.push_back(param);
	}

	dsp->setActive(dspObject->active);
	dsp->setBypass(dspObject->bypass);

	dspObject->dspType = dspType;
	dspObject->dsp = dsp;

	return dspObject;
}

FMOD::Sound* SoundManager::CreateSound3D(const std::string& name, const std::string& path, const glm::vec3& position)
{
	std::unordered_map<std::string, FMOD::Sound*>::iterator it = sounds.find(name);
	if (it != sounds.end())
	{
		std::cout << "A sound already exists with name '" << name << "'!" << std::endl;
		return nullptr;
	}

	FMOD::Sound* sound = nullptr;

	result = system->createSound(path.c_str(), FMOD_3D, 0, &sound);
	if (result != FMOD_OK)
	{
		std::cout << "Failed to create 3D sound! (FMOD)" << std::endl;
		return NULL;
	}

	sounds.insert({ name, sound });

	return sound;
}

FMOD::Channel* SoundManager::PlaySound(const std::string& name, const PlaySoundInfo& info)
{
	std::unordered_map<std::string, FMOD::Sound*>::iterator it = sounds.find(name);
	if (it == sounds.end()) return nullptr;
	
	return PlaySound(it->second, info);
}

FMOD::Channel* SoundManager::PlaySound(FMOD::Sound* sound, const PlaySoundInfo& info)
{
	if (!sound) return nullptr;

	FMOD::Channel* channel = nullptr;
	system->playSound(sound, nullptr, info.startPaused, &channel);

	channel->setVolume(info.volume);

	// Add DPSs
	for (DSPObject* dsp : info.dsps)
	{
		channel->addDSP(FMOD_CHANNELCONTROL_DSP_TAIL, dsp->dsp);
	}

	return channel;
}