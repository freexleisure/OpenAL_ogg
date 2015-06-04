#ifndef AUDIO_AUIDOINTERFACEMANGE_H__
#define AUDIO_AUIDOINTERFACEMANGE_H__


#ifdef _WIN32
#ifdef AUDIOINTERFACE_DLL
#define AUIDO_API _declspec(dllexport)
#else
#define AUIDO_API _declspec(dllimport)
#endif
#endif


class BridgeSourceClass;
class AUIDO_API CAudioInterfaceManage
{
private:
	CAudioInterfaceManage(void);
	~CAudioInterfaceManage(void);
	CAudioInterfaceManage(const CAudioInterfaceManage&);
	CAudioInterfaceManage& operator=(const CAudioInterfaceManage&);

public:
	static CAudioInterfaceManage* getInstance();
	void Destory_All_Audio_Source();

	
	/**
	* pFileName:文件名 Res/sss.mp3, xxx.mp3等，标示资源查找
	* pAbsolutePath:绝对路径 D:/Cardgame/Exe/Client/Res/xx.mp3
	* 播放2d音频文件
	*/
	bool OpenAl_Play_Audio_2d(const char* pFileName,const char* pAbsolutePath,bool loop=false);
	bool OpenAl_Play_Audio_3d(const char* pFileName,const char* pAbsolutePath,float x,float y,float z,bool loop=false);
	
	/**OpenAl_Stop_Audio
	*Release Memory Audio Data
	*/
	bool OpenAl_Stop_Audio(const char* pFileName);
	bool OpenAl_Pause_Audio(const char* pFileName);
	bool OpenAl_Resume_Audio(const char* pFileName);

	/**
	*preload audio data
	*/
	bool OpenAl_Preload_Audio(const char* pFileName,const char* pAbsolutePath);

	/**
	*All audio stop                                                                    
	*/
	bool OpenAl_Set_All_Audio_Pause();


	bool OpenAl_Is_Audio_Playing(const char* pFileName);
	bool OpenAl_Is_Audio_Paused(const char* pFileName);
	bool OpenAl_Is_Audio_Stopped(const char* pFileName);
	bool OpenAl_Is_Loop_Play(const char* pFileName);

	/**
	* Indicate the gain (volume amplification) applied. 
	* Type:   ALfloat.
	* Range:  ]0.0-  ]
	* A value of 1.0 means un-attenuated/unchanged.
	* Each division by 2 equals an attenuation of -6dB.
	* Each multiplicaton with 2 equals an amplification of +6dB.
	* A value of 0.0 is meaningless with respect to a logarithmic
	*  scale; it is interpreted as zero volume - the channel
	*  is effectively disabled.
	*/
	bool OpenAl_Set_Audio_Volume(const char* pFileName,float fVolume);
	bool OpenAl_Get_Audio_Volume(const char* pFileName,float& fVolume);
	bool OpenAl_Get_Range_Volume(const char* pFileName,float& MinVolume,float& MaxVolume);


	/**
	* Directional source, outer cone gain.
	*
	* Default:  0.0
	* Range:    [0.0 - 1.0]
	* Logarithmic
	* 外锥(outercone)外的增益
	*/
	bool OpenAl_Set_Audio_Cone_Outer_Volume(const char* pFileName,float fVolume);
	bool OpenAl_Get_Audio_Cone_Outer_Volume(const char* pFileName,float &fVolume);

	/**
	* Specify the pitch to be applied, either at source,
	*  or on mixer results, at listener.
	* Range:   [0.5-2.0]
	* Default: 1.0
	*/
	bool OpenAl_Set_Audio_Pitch(const char* pFileName,float fPitch);
	bool OpenAl_Get_Audio_Pitch(const char* pFileName,float &fPitch);

	bool OpenAl_Audio_Move(const char* pFileName,float x,float y,float z);

	/** 
	* Specify the current location in three dimensional space.
	* OpenAL, like OpenGL, uses a right handed coordinate system,
	*  where in a frontal default view X (thumb) points right, 
	*  Y points up (index finger), and Z points towards the
	*  viewer/camera (middle finger). 
	* To switch from a left handed coordinate system, flip the
	*  sign on the Z coordinate.
	* Listener position is always in the world coordinate system.
	*/ 
	bool OpenAl_Set_Audio_Position(const char* pFileName,float x,float y,float z);
	bool OpenAl_Get_Audio_Position(const char* pFileName,float& x,float& y,float& z);

	bool OpenAl_Set_Audio_Velocity(const char* pFileName,float x,float y,float z);
	bool OpenAl_Get_Audio_Velocity(const char* pFileName,float& x,float& y,float& z);

	bool OpenAl_Set_Audio_Direction(const char* pFileName,int x,int y,int z);
	bool OpenAl_Get_Audio_Direction(const char* pFileName,int& x,int& y,int& z);

	bool OpenAl_Set_Audio_Rolloff_Factor(const char* pFileName,float factor);
	bool OpenAl_Get_Audio_Rolloff_Factor(const char* pFileName,float &factor);

	bool OpenAl_Set_Audio_Reference_Distance(const char* pFileName,float refDis);
	bool OpenAl_Get_Audio_Reference_Distance(const char* pFileName,float &refDis);

	bool OpenAl_Set_Audio_Max_Distance(const char* pFileName,float maxDis);
	bool OpenAl_Get_Audio_Max_Distance(const char* pFileName,float &maxDis);
	/**
	* Directional source, inner cone angle, in degrees.
	* Range:    [0-360] 
	* Default:  360
	* 内锥覆盖的角度，在此之中，source不会衰减
	*/
	bool OpenAl_Set_Audio_Cone_Inner_Angle(const char* pFileName,int nAngle);
	bool OpenAl_Get_Audio_Cone_Inner_Angle(const char* pFileName,int &nAngle);

	/**
	* Directional source, inner cone angle, in degrees.
	* Range:    [0-360] 
	* Default:  360
	* 外锥覆盖的角度，在此之外，source完全衰减。内锥和外锥之间平滑衰减
	*/
	bool OpenAl_Set_Audio_Cone_Outer_Angle(const char* pFileName,int nAngle);
	bool OpenAl_Get_Audio_Cone_Outer_Angle(const char* pFileName,int &nAngle);

	/** Global tweakage. */

	/**
	* Doppler scale.  Default 1.0
	* 全局多普勒系数
	*/
	bool OpenAl_Set_Audio_Doppler_Factor(const char* pFileName,float factor);
	bool OpenAl_Get_Audio_Doppler_Factor(const char* pFileName,float &factor);


	/**
	* Tweaks speed of propagation.
	* 
	*/
	bool OpenAl_Set_Audio_Doppler_Velocity(const char* pFileName,float Vel);
	bool OpenAl_Get_Audio_Doppler_Velocity(const char* pFileName,float &Vel);

	/**
	* Speed of Sound in units per second
	*/
	bool OpenAl_Set_Audio_Speed_Of_Sound(const char* pFileName,float speed);
	bool OpenAl_Get_Audio_Speed_Of_Sound(const char* pFileName,float &speed);

	/**
	*
	*/
	bool OpenAl_Set_Listener_Direction(float Forward[3],float Up[3]);
	bool OpenAl_Set_Listener_Position(float x,float y,float z);
	bool OpenAl_Set_Listener_Velocity(float x,float y,float z);
	bool OpenAl_Set_Listener_Volume(float fVolume);


private:
	BridgeSourceClass *_pBridgeSourceClass;
};


#endif
