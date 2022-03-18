#pragma once

#include <FMOD/fmod.hpp>
#include <yaml-cpp/yaml.h>

#include <vector>

struct DSPParameter
{
	DSPParameter(FMOD_DSP_PARAMETER_DESC* parameterDescription) : parameterDescription(parameterDescription)
	{
		if (parameterDescription->type == FMOD_DSP_PARAMETER_TYPE_FLOAT)
		{
			this->fValue = parameterDescription->floatdesc.defaultval;
		}
		else if (parameterDescription->type == FMOD_DSP_PARAMETER_TYPE_INT)
		{
			this->iValue = parameterDescription->intdesc.defaultval;
		}
		else if (parameterDescription->type == FMOD_DSP_PARAMETER_TYPE_BOOL)
		{
			this->bValue = parameterDescription->booldesc.defaultval;
		}
	}

	FMOD_DSP_PARAMETER_DESC* parameterDescription;

	float fValue = 0.0f;
	int iValue = 1;
	bool bValue = false;
};

class DSPObject
{
public:
	~DSPObject();

	void UpdateParameters();

	FMOD_DSP_TYPE dspType;
	FMOD::DSP* dsp = nullptr;
	bool active = true;
	bool bypass = true;

	std::vector<DSPParameter> parameterDescriptions;
};