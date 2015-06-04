
#ifndef AUDIO_AUDIOSOURCE_H__
#define AUDIO_AUDIOSOURCE_H__

#include "OggDecode.h"
#include <atomic>
#include <mutex>


class CAudioSource
{
public:
	CAudioSource(const char *pFileName);
	~CAudioSource();

public:
	bool Is_Loop_Play(){return _Loop.load();}

	bool Set_Audio_Volume(ALfloat fVolume);
	bool Get_Audio_Volume(ALfloat& fVolume);
	bool Get_Range_Volume(ALfloat& MinVolume,ALfloat& MaxVolume);


	/**
	* Directional source, outer cone gain.
	*
	* Default:  0.0
	* Range:    [0.0 - 1.0]
	* Logarithmic
	* 外锥(outercone)外的增益
	*/
	bool Set_Audio_Cone_Outer_Volume(ALfloat fVolume);
	bool Get_Audio_Cone_Outer_Volume(ALfloat &fVolume);


	bool Set_Audio_Pitch(ALfloat fPitch);
	bool Get_Audio_Pitch(ALfloat &fPitch);

	bool Is_Audio_Playing();
	bool Is_Audio_Paused();
	bool Is_Audio_Stopped();
	

	bool Audio_Play();
	bool Audio_Stop();
	bool Audio_Pause();
	bool Audio_Play2d(bool bLoop);
	
	bool Audio_Play3d(ALfloat x,ALfloat y,ALfloat z,bool bLoop);
	bool Audio_Move(ALfloat x,ALfloat y,ALfloat z);

	bool Set_Audio_Position(ALfloat x,ALfloat y,ALfloat z);
	bool Get_Audio_Position(ALfloat& x,ALfloat& y,ALfloat& z);

	bool Set_Audio_Velocity(ALfloat x,ALfloat y,ALfloat z);
	bool Get_Audio_Velocity(ALfloat& x,ALfloat& y,ALfloat& z);

	bool Set_Audio_Direction(ALint x,ALint y,ALint z);
	bool Get_Audio_Direction(ALint& x,ALint& y,ALint& z);

	bool Set_Audio_Rolloff_Factor(ALfloat factor);
	bool Get_Audio_Rolloff_Factor(ALfloat &factor);

	bool Set_Audio_Reference_Distance(ALfloat refDis);
	bool Get_Audio_Reference_Distance(ALfloat &refDis);

	bool Set_Audio_Max_Distance(ALfloat maxDis);
	bool Get_Audio_Max_Distance(ALfloat &maxDis);
	/**
	* Directional source, inner cone angle, in degrees.
	* Range:    [0-360] 
	* Default:  360
	* 内锥覆盖的角度，在此之中，source不会衰减
	*/
	bool Set_Audio_Cone_Inner_Angle(ALint nAngle);
	bool Get_Audio_Cone_Inner_Angle(ALint &nAngle);

	/**
	* Directional source, inner cone angle, in degrees.
	* Range:    [0-360] 
	* Default:  360
	* 外锥覆盖的角度，在此之外，source完全衰减。内锥和外锥之间平滑衰减
	*/
	bool Set_Audio_Cone_Outer_Angle(ALint nAngle);
	bool Get_Audio_Cone_Outer_Angle(ALint &nAngle);

	/** Global tweakage. */

	/**
	* Doppler scale.  Default 1.0
	* 全局多普勒系数
	*/
	bool Set_Audio_Doppler_Factor(ALfloat factor);
	bool Get_Audio_Doppler_Factor(ALfloat &factor);


	/**
	* Tweaks speed of propagation.
	* 
	*/
	bool Set_Audio_Doppler_Velocity(ALfloat Vel);
	bool Get_Audio_Doppler_Velocity(ALfloat &Vel);

	/**
	* Speed of Sound in units per second
	*/
	bool Set_Audio_Speed_Of_Sound(ALfloat speed);
	bool Get_Audio_Speed_Of_Sound(ALfloat &speed);

	bool Update_Data_Stream();

	bool Preload_Audio_Data();
	void Destory_Config_OpenAl_Sources();

protected:
	bool Read_Data_Stream(ALuint buff);
	bool Config_OpenAl_Sources();


protected:
	ALCcontext *_pCurAlcContext;

	ALuint _AudioSource;
	ALuint* _AudioBuffer;
	int _nArraySize;
	COggDecode _OggDecode;

	std::atomic<bool> _Loop;

	std::recursive_mutex _Lock;

	std::atomic<bool> _preload;
};

#endif