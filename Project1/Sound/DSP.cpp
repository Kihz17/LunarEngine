#include "DSP.h"
#include "SoundManager.h"

DSPObject::~DSPObject()
{
	dsp->release();
}

void DSPObject::UpdateParameters()
{
	for (int i = 0; i < parameterDescriptions.size(); i++)
	{
		const DSPParameter& param = parameterDescriptions[i];
		FMOD_DSP_PARAMETER_DESC* parameterDescription = param.parameterDescription;

		if (parameterDescription->type == FMOD_DSP_PARAMETER_TYPE_FLOAT)
		{
			dsp->setParameterFloat(i, param.fValue);
		}
		else if (parameterDescription->type == FMOD_DSP_PARAMETER_TYPE_INT)
		{
			dsp->setParameterInt(i, param.iValue);
		}
		else if (parameterDescription->type == FMOD_DSP_PARAMETER_TYPE_BOOL)
		{
			dsp->setParameterBool(i, param.bValue);
		}
	}

	dsp->setActive(this->active);
	dsp->setBypass(this->bypass);
}