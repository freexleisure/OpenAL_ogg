#include "stdafx.h"
#include "AudioDevice.h"
#include <assert.h>



CAudioDevice::CAudioDevice(void)
	:_pAlcDevice(0)
{
}


CAudioDevice::~CAudioDevice(void)
{
}

bool CAudioDevice::Create_OpenAL_Device()
{
	_pAlcDevice = alcOpenDevice(0);
	if (!_pAlcDevice)
	{
		OpenAL_Check_Error();
		return false;
	}

	ALCcontext *pAlcContext = alcCreateContext(_pAlcDevice,0);
	if (!pAlcContext)
	{
		OpenAL_Check_Error();
		goto CLOSEDEVICE;
	}

	if(! alcMakeContextCurrent(pAlcContext))
	{
		OpenAL_Check_Error();
		goto DESTORYCONTEXT;
	}

	return true;

DESTORYCONTEXT:
	alcDestroyContext(pAlcContext);

CLOSEDEVICE:
	alcCloseDevice(_pAlcDevice);
	_pAlcDevice = 0;

	return false;
}

void CAudioDevice::Destory_OpenAL_Device()
{
	if (_pAlcDevice)
	{
		alcMakeContextCurrent(NULL);
		ALCcontext *pAlcContext = alcGetCurrentContext();
		if (pAlcContext)
		{
			ALCdevice *pAlcDevice = alcGetContextsDevice(pAlcContext);
			assert(pAlcDevice == _pAlcDevice);
			alcDestroyContext(pAlcContext);
			alcCloseDevice(_pAlcDevice);
			OpenAL_Check_Error();
		}
		_pAlcDevice = 0;
	}
}
