#include "stdafx.h"
#include "Listener.h"


CListener::CListener(void)
{
}


CListener::~CListener(void)
{
}

bool CListener::Set_Listener_Direction(ALfloat Forward[3],ALfloat Up[3])
{
	std::unique_lock<std::mutex> lock(_mutex);
	ALfloat Dir[] = {Forward[0],Forward[1],Forward[2],Up[0],Up[1],Up[2]};
	alListenerfv(AL_DIRECTION,Dir);

	return !OpenAL_Check_Error();
}

bool CListener::Set_Listener_Position(ALfloat x,ALfloat y,ALfloat z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	alListener3f(AL_POSITION,x,y,z);

	return !OpenAL_Check_Error();
}

bool CListener::Set_Listener_Velocity(ALfloat x,ALfloat y,ALfloat z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	alListener3f(AL_VELOCITY,x,y,z);

	return !OpenAL_Check_Error();
}

bool CListener::Set_Listener_Volume(ALfloat fVolume)
{
	std::unique_lock<std::mutex> lock(_mutex);
	alListenerf(AL_GAIN,fVolume);

	return !OpenAL_Check_Error();
}