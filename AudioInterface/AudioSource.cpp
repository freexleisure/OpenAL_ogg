

#include "stdafx.h"
#include "AudioSource.h"



#define DATA_BUFF_SIZE 64*1024



CAudioSource::CAudioSource(const char *pFileName)
	:_AudioSource(0),
	_nArraySize(0),
	_OggDecode(pFileName),
	_Loop(false),
	_preload(false)
{
	_pCurAlcContext = alcGetCurrentContext();
	if(!_pCurAlcContext)
	{
		OpenAL_Check_Error();
	}

#ifdef DATA_STREAM_LOAD
	_nArraySize = 10;
#else
	_nArraySize = 1;
#endif
	_AudioBuffer = new ALuint[_nArraySize];
	memset(_AudioBuffer,0,_nArraySize*sizeof(ALuint));

	if(!Config_OpenAl_Sources())
		_pCurAlcContext = 0;
}


CAudioSource::~CAudioSource()
{
	if(_AudioBuffer)
		delete []_AudioBuffer;
}

bool CAudioSource::Config_OpenAl_Sources()
{
	alGenSources(1,&_AudioSource);
	alGenBuffers(_nArraySize,_AudioBuffer);
	Set_Audio_Volume(1.0f);
	Set_Audio_Pitch(1.0f);
	
	//alSourcei(_AudioSource,AL_SOURCE_TYPE,AL_STREAMING);

	return !OpenAL_Check_Error();
}

void CAudioSource::Destory_Config_OpenAl_Sources()
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	_pCurAlcContext = 0;

	alSourceStop(_AudioSource);
	ALint nQueueS(0); 
	alGetSourcei(_AudioSource,AL_BUFFERS_PROCESSED,&nQueueS);
	if(nQueueS)for (int n=0; n<nQueueS; ++n)
	{
		ALuint buff(0);
		alSourceUnqueueBuffers(_AudioSource,1,&buff);
	}

	alDeleteSources(1,&_AudioSource);
	alDeleteBuffers(_nArraySize,_AudioBuffer);
	_AudioSource = 0;

	OpenAL_Check_Error();
}

bool CAudioSource::Read_Data_Stream(ALuint sourcebuff)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if (_AudioSource)
	{
		long nReadSize=0;

#ifdef DATA_STREAM_LOAD
		char buff[DATA_BUFF_SIZE];
		while (nReadSize<DATA_BUFF_SIZE)
		{
			long nSize = _OggDecode.Read_Ogg_File_Data(buff+nReadSize,DATA_BUFF_SIZE-nReadSize);
			if (nSize < 0)
			{
				Audio_Stop();
				return false;
			}
			else if (nSize == 0)
			{
				if(_Loop.load())
				{
					_OggDecode.Set_Ogg_File_Pos(0);
				}
				break;
			}

			nReadSize += nSize;
		}
#endif
#ifdef DATA_ALL_LOAD
		long nFileSize = _OggDecode.get_File_Size();
		std::unique_ptr<char[]> pBuff(new char[nFileSize]);
		char *buff = pBuff.get();
		while (nReadSize<nFileSize)
		{
			long nSize = _OggDecode.Read_Ogg_File_Data(buff+nReadSize,nFileSize-nReadSize);
			if (nSize < 0)
			{
				Audio_Stop();
				return false;
			}
			else if (nSize == 0)
			{
				if(_Loop.load())
				{
					_OggDecode.Set_Ogg_File_Pos(0);
				}
				break;
			}

			nReadSize += nSize;
		}
#endif


		if(nReadSize)
		{
			alBufferData(sourcebuff,_OggDecode.get_Ogg_Format(),buff,nReadSize,_OggDecode.get_Ogg_Frequency());

			return !OpenAL_Check_Error();
		}
	}
	return false;
}


bool CAudioSource::Update_Data_Stream()
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(!_AudioSource)
		return false;

	if(Is_Audio_Playing())
	{
		ALint nQueueS(0); 
		alGetSourcei(_AudioSource,AL_BUFFERS_PROCESSED,&nQueueS);
		OpenAL_Check_Error();
		if(nQueueS)
		{
			for (int n=0; n<nQueueS; ++n)
			{
				ALuint buff(0);
				alSourceUnqueueBuffers(_AudioSource,1,&buff);

				if(OpenAL_Check_Error())
				{
					n = --n<0?0:n;
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					continue;
				}

				if (Read_Data_Stream(buff))
				{
					alSourceQueueBuffers(_AudioSource,1,&buff);

					if(OpenAL_Check_Error())
					{
						n = --n<0?0:n;
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						continue;
					}
				}
			}
		}

		OpenAL_Check_Error();
	}
	return true;
}


bool CAudioSource::Preload_Audio_Data()
{
#ifdef DATA_ALL_LOAD
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if (!_AudioSource)
	{
		return false;
	}

	bool bIsPlaying = Is_Audio_Playing();

	if (!Is_Audio_Paused() && !bIsPlaying)
	{
		alSourcef(_AudioSource,AL_BUFFER,0);
		OpenAL_Check_Error();

		int nQueues(0);
		for (ALuint n=0; n<_nArraySize; ++n)
		{
			if(Read_Data_Stream(_AudioBuffer[n]))
				++nQueues;
		}

		if(nQueues)
		{
			_preload.store(true);
			alSourceQueueBuffers(_AudioSource,nQueues,_AudioBuffer);
		}
	}

	return !OpenAL_Check_Error();
#else
	return false;
#endif
}


bool CAudioSource::Set_Audio_Volume(ALfloat fVolume)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(_AudioSource)
	{
		alSourcef(_AudioSource,AL_GAIN,fVolume);

		return !OpenAL_Check_Error();
	}

	return false;
}

bool CAudioSource::Get_Audio_Volume(ALfloat& fVolume)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(_AudioSource)
	{
		alGetSourcef(_AudioSource,AL_GAIN,&fVolume);

		return !OpenAL_Check_Error();
	}
	return false;
}

bool CAudioSource::Get_Range_Volume(ALfloat& MinVolume,ALfloat& MaxVolume)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(_AudioSource)
	{
		alGetSourcef(_AudioSource,AL_MIN_GAIN,&MinVolume);
		alGetSourcef(_AudioSource,AL_MAX_GAIN,&MaxVolume);
		return !OpenAL_Check_Error();
	}
	return false;
}

bool CAudioSource::Set_Audio_Cone_Outer_Volume(ALfloat fVolume)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcef(_AudioSource,AL_CONE_OUTER_GAIN,fVolume);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Cone_Outer_Volume(ALfloat &fVolume)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcef(_AudioSource,AL_CONE_OUTER_GAIN,&fVolume);

	return !OpenAL_Check_Error();
}


bool CAudioSource::Set_Audio_Pitch(ALfloat fPitch)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(_AudioSource)
	{
		alSourcef(_AudioSource,AL_PITCH,fPitch);
		return !OpenAL_Check_Error();
	}
	return false;
}

bool CAudioSource::Get_Audio_Pitch(ALfloat &fPitch)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(_AudioSource)
	{
		alGetSourcef(_AudioSource,AL_PITCH,&fPitch);
		return !OpenAL_Check_Error();
	}
	return false;
}

bool CAudioSource::Is_Audio_Playing()
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(_AudioSource)
	{
		ALint state;
		alGetSourcei(_AudioSource,AL_SOURCE_STATE,&state);
		OpenAL_Check_Error();

		return state == AL_PLAYING;
	}
	return false;
}

bool CAudioSource::Is_Audio_Paused()
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(_AudioSource)
	{
		ALint state;
		alGetSourcei(_AudioSource,AL_SOURCE_STATE,&state);
		OpenAL_Check_Error();

		return state == AL_PAUSED;
	}
	return false;
}

bool CAudioSource::Is_Audio_Stopped()
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if(_AudioSource)
	{
		ALint state;
		alGetSourcei(_AudioSource,AL_SOURCE_STATE,&state);
		OpenAL_Check_Error();

		return state == AL_STOPPED;
	}
	return false;
}

bool CAudioSource::Audio_Play()
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);

	if (!_AudioSource)
	{
		return false;
	}
	
	bool bIsPlaying = Is_Audio_Playing();

	if (!Is_Audio_Paused() && !bIsPlaying)
	{
		alSourcef(_AudioSource,AL_BUFFER,0);
		OpenAL_Check_Error();

		int nQueues(0);
		for (ALuint n=0; n<_nArraySize; ++n)
		{
			if(Read_Data_Stream(_AudioBuffer[n]))
				++nQueues;
		}

		if(nQueues)
		{
			alSourceQueueBuffers(_AudioSource,nQueues,_AudioBuffer);
		}
		OpenAL_Check_Error();
	}

	if( !bIsPlaying)
		alSourcePlay(_AudioSource);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Audio_Stop()
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if (!_AudioSource)
	{
		return false;
	}

	alSourceStop(_AudioSource);
	ALint nQueueS(0); 
	alGetSourcei(_AudioSource,AL_BUFFERS_QUEUED,&nQueueS);
	if(nQueueS)
	{
		ALuint buff(0);
		alSourceUnqueueBuffers(_AudioSource,nQueueS,&buff);
	}

	return !OpenAL_Check_Error();
}

bool CAudioSource::Audio_Pause()
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if (!_AudioSource)
	{
		return false;
	}

	alSourcePause(_AudioSource);
	return !OpenAL_Check_Error();
}


bool CAudioSource::Audio_Play2d(bool bLoop)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if (!_AudioSource)
	{
		return false;
	}

	_Loop.store(bLoop);

	alSourcei(_AudioSource,AL_SOURCE_RELATIVE,true);
#ifdef DATA_ALL_LOAD
	alSourcei(_AudioSource,AL_LOOPING, bLoop?AL_TRUE:AL_FALSE);

	if(!_preload.load())
	{
		return Audio_Play();
	}
	else
	{
		alSourcePlay(_AudioSource);
	}
	return !OpenAL_Check_Error();
#else
	return Audio_Play();
#endif
}

bool CAudioSource::Audio_Play3d(ALfloat x,ALfloat y,ALfloat z,bool bLoop)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSource3f(_AudioSource,AL_POSITION,x,y,z);
	alSourcei(_AudioSource,AL_SOURCE_RELATIVE,true);
	_Loop.store(bLoop);

#ifdef DATA_ALL_LOAD
	alSourcei(_AudioSource,AL_LOOPING, bLoop?AL_TRUE:AL_FALSE);

	if(!_preload.load())
	{
		return Audio_Play();
	}
	else
	{
		alSourcePlay(_AudioSource);
	}
#else
	return Audio_Play();
#endif
	return !OpenAL_Check_Error();
}

bool CAudioSource::Audio_Move(ALfloat x,ALfloat y,ALfloat z)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	ALfloat ox=0,oy=0,oz=0;

	alGetSource3f(_AudioSource,AL_POSITION,&ox,&oy,&oz);

	alSource3f(_AudioSource,AL_VELOCITY,ox-x,oy-y,oz-z);
	alSource3f(_AudioSource,AL_POSITION,x,y,z);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Position(ALfloat x,ALfloat y,ALfloat z)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSource3f(_AudioSource,AL_POSITION,x,y,z);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Position(ALfloat& x,ALfloat& y,ALfloat& z)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSource3f(_AudioSource,AL_POSITION,&x,&y,&z);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Velocity(ALfloat x,ALfloat y,ALfloat z)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSource3f(_AudioSource,AL_VELOCITY,x,y,z);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Velocity(ALfloat& x,ALfloat& y,ALfloat& z)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSource3f(_AudioSource,AL_VELOCITY,&x,&y,&z);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Direction(ALint x,ALint y,ALint z)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSource3i(_AudioSource,AL_VELOCITY,x,y,z);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Direction(ALint& x,ALint& y,ALint& z)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSource3i(_AudioSource,AL_VELOCITY,&x,&y,&z);

	return !OpenAL_Check_Error();
}


bool CAudioSource::Set_Audio_Rolloff_Factor(ALfloat factor)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcef(_AudioSource,AL_ROLLOFF_FACTOR,factor);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Rolloff_Factor(ALfloat& factor)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcef(_AudioSource,AL_ROLLOFF_FACTOR,&factor);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Reference_Distance(ALfloat refDis)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcef(_AudioSource,AL_ROLLOFF_FACTOR,refDis);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Reference_Distance(ALfloat &refDis)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcef(_AudioSource,AL_ROLLOFF_FACTOR,&refDis);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Max_Distance(ALfloat maxDis)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcef(_AudioSource,AL_MAX_DISTANCE,maxDis);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Max_Distance(ALfloat &maxDis)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcef(_AudioSource,AL_MAX_DISTANCE,&maxDis);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Cone_Inner_Angle(ALint nAngle)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcei(_AudioSource,AL_CONE_INNER_ANGLE,nAngle);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Cone_Inner_Angle(ALint &nAngle)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcei(_AudioSource,AL_CONE_INNER_ANGLE,&nAngle);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Cone_Outer_Angle(ALint nAngle)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcei(_AudioSource,AL_CONE_INNER_ANGLE,nAngle);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Cone_Outer_Angle(ALint &nAngle)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcei(_AudioSource,AL_CONE_INNER_ANGLE,&nAngle);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Doppler_Factor(ALfloat factor)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcef(_AudioSource,AL_DOPPLER_FACTOR,factor);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Doppler_Factor(ALfloat &factor)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcef(_AudioSource,AL_DOPPLER_FACTOR,&factor);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Doppler_Velocity(ALfloat Vel)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcef(_AudioSource,AL_DOPPLER_VELOCITY,Vel);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Get_Audio_Doppler_Velocity(ALfloat &Vel)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcef(_AudioSource,AL_DOPPLER_VELOCITY,&Vel);

	return !OpenAL_Check_Error();
}

bool CAudioSource::Set_Audio_Speed_Of_Sound(ALfloat speed)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alSourcef(_AudioSource,AL_SPEED_OF_SOUND,speed);
	return !OpenAL_Check_Error();
}

bool  CAudioSource::Get_Audio_Speed_Of_Sound(ALfloat &speed)
{
	std::unique_lock<std::recursive_mutex> lock(_Lock);
	if(!_AudioSource)
		return false;

	alGetSourcef(_AudioSource,AL_SPEED_OF_SOUND,&speed);
	return !OpenAL_Check_Error();
}









