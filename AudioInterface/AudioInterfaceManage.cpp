#include "stdafx.h"
#define AUDIOINTERFACE_DLL
#include "AudioInterfaceManage.h"
#include "AudioDevice.h"
#include "AudioSource.h"
#include <unordered_map>
#include "Listener.h"
#include <condition_variable>

class cmpFun
{
public:
	bool operator()(const std::string& str1,const std::string& str2)
	{
		return strcmp(str1.c_str(),str2.c_str()) == 0;
	}
};

class hashFun
{
public:
	size_t operator()(const std::string& Keyval)
	{
		return std::hash_value(Keyval);
	}
};

class BridgeSourceClass
{
public:
	BridgeSourceClass();
	~BridgeSourceClass();

public:
#ifdef DATA_STREAM_LOAD
	void Update(BridgeSourceClass *pObj);
#endif

	void Insert_Audio_Source(const char *pFileName,CAudioSource *pNewSource);
	void Erase_Audio_Source(const char* pFileName);
	void Clear_Audio_Source();
	CAudioSource* Find_Audio_Source(const char *pFileName);

	bool Play_Audio_2d(const char* pFileName,const char* pAbsolutePath,bool loop=false);
	bool Play_Audio_3d(const char* pFileName,const char* pAbsolutePath,float x,float y,float z,bool loop=false);
	bool Stop_Audio(const char* pFileName);
	bool Pause_Audio(const char* pFileName);
	bool Resume_Audio(const char* pFileName);

	bool Preload_Audio(const char* pFileName,const char* pAbsolutePath);


	bool Is_Audio_Playing(const char* pFileName);
	bool Is_Audio_Paused(const char* pFileName);
	bool Is_Audio_Stopped(const char* pFileName);
	bool Is_Loop_Play(const char* pFileName);

	bool Set_Audio_Volume(const char* pFileName,float fVolume);
	bool Get_Audio_Volume(const char* pFileName,float& fVolume);
	bool Get_Range_Volume(const char* pFileName,float& MinVolume,float& MaxVolume);


	/**
	* Directional source, outer cone gain.
	*
	* Default:  0.0
	* Range:    [0.0 - 1.0]
	* Logarithmic
	* 外锥(outercone)外的增益
	*/
	bool Set_Audio_Cone_Outer_Volume(const char* pFileName,float fVolume);
	bool Get_Audio_Cone_Outer_Volume(const char* pFileName,float &fVolume);


	bool Set_Audio_Pitch(const char* pFileName,float fPitch);
	bool Get_Audio_Pitch(const char* pFileName,float &fPitch);

	bool Audio_Move(const char* pFileName,float x,float y,float z);

	bool Set_Audio_Position(const char* pFileName,float x,float y,float z);
	bool Get_Audio_Position(const char* pFileName,float& x,float& y,float& z);

	bool Set_Audio_Velocity(const char* pFileName,float x,float y,float z);
	bool Get_Audio_Velocity(const char* pFileName,float& x,float& y,float& z);

	bool Set_Audio_Direction(const char* pFileName,int x,int y,int z);
	bool Get_Audio_Direction(const char* pFileName,int& x,int& y,int& z);

	bool Set_Audio_Rolloff_Factor(const char* pFileName,float factor);
	bool Get_Audio_Rolloff_Factor(const char* pFileName,float &factor);

	bool Set_Audio_Reference_Distance(const char* pFileName,float refDis);
	bool Get_Audio_Reference_Distance(const char* pFileName,float &refDis);

	bool Set_Audio_Max_Distance(const char* pFileName,float maxDis);
	bool Get_Audio_Max_Distance(const char* pFileName,float &maxDis);
	/**
	* Directional source, inner cone angle, in degrees.
	* Range:    [0-360] 
	* Default:  360
	* 内锥覆盖的角度，在此之中，source不会衰减
	*/
	bool Set_Audio_Cone_Inner_Angle(const char* pFileName,int nAngle);
	bool Get_Audio_Cone_Inner_Angle(const char* pFileName,int &nAngle);

	/**
	* Directional source, inner cone angle, in degrees.
	* Range:    [0-360] 
	* Default:  360
	* 外锥覆盖的角度，在此之外，source完全衰减。内锥和外锥之间平滑衰减
	*/
	bool Set_Audio_Cone_Outer_Angle(const char* pFileName,int nAngle);
	bool Get_Audio_Cone_Outer_Angle(const char* pFileName,int &nAngle);

	/** Global tweakage. */

	/**
	* Doppler scale.  Default 1.0
	* 全局多普勒系数
	*/
	bool Set_Audio_Doppler_Factor(const char* pFileName,float factor);
	bool Get_Audio_Doppler_Factor(const char* pFileName,float &factor);


	/**
	* Tweaks speed of propagation.
	* 
	*/
	bool Set_Audio_Doppler_Velocity(const char* pFileName,float Vel);
	bool Get_Audio_Doppler_Velocity(const char* pFileName,float &Vel);

	/**
	* Speed of Sound in units per second
	*/
	bool Set_Audio_Speed_Of_Sound(const char* pFileName,float speed);
	bool Get_Audio_Speed_Of_Sound(const char* pFileName,float &speed);

	void Set_All_Audio_Pause();
private:
	std::unique_ptr<CAudioDevice> _OpenALAudioDevice;

#ifdef DATA_STREAM_LOAD
	std::thread _thread;
	std::atomic<bool> _loop;
	std::condition_variable _cond;
	std::mutex _condMutex;
#endif

	std::unordered_map<std::string,CAudioSource*,hashFun,cmpFun> _pManageAudioSource;
	std::mutex _mutex;

	CListener _listener;

	friend class CAudioInterfaceManage;
};

BridgeSourceClass::BridgeSourceClass()
	:_OpenALAudioDevice(new CAudioDevice())
{
	bool bRet = _OpenALAudioDevice->Create_OpenAL_Device();
#ifdef DATA_STREAM_LOAD
	if(bRet)
	{
		_loop.store(true);
		_thread = std::thread(std::mem_fn(&BridgeSourceClass::Update),this,this);
	}
#endif
}

BridgeSourceClass::~BridgeSourceClass()
{
#ifdef DATA_STREAM_LOAD
	_loop.store(false);
	_thread.join();
#endif
	_OpenALAudioDevice->Destory_OpenAL_Device();
}

#ifdef DATA_STREAM_LOAD
void BridgeSourceClass::Update(BridgeSourceClass *pObj)
{
	for (;pObj->_loop.load();)
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);
			for(auto it = _pManageAudioSource.begin(); it != _pManageAudioSource.end(); ++it)
			{
				it->second->Update_Data_Stream();
			}
		}

		std::unique_lock<std::mutex> condLock(_condMutex);
		pObj->_cond.wait_for(condLock,std::chrono::milliseconds(3000),[pObj](){return !pObj->_loop.load();});
	}
}
#endif

void BridgeSourceClass::Insert_Audio_Source(const char *pFileName,CAudioSource *pNewSource)
{
	_pManageAudioSource.insert(std::make_pair(std::string(pFileName),pNewSource));
}

void BridgeSourceClass::Erase_Audio_Source(const char* pFileName)
{
	auto it = _pManageAudioSource.find(std::string(pFileName));
	if (it != _pManageAudioSource.end())
	{
		CAudioSource *pDelSource = it->second;
		_pManageAudioSource.erase(std::string(pFileName));
		pDelSource->Destory_Config_OpenAl_Sources();
		delete pDelSource;
	}
}

void BridgeSourceClass::Clear_Audio_Source()
{
	std::unique_lock<std::mutex> lock(_mutex);
	for (auto const &it:_pManageAudioSource)
	{
		CAudioSource *pDelSource = it.second;
		pDelSource->Destory_Config_OpenAl_Sources();
		delete pDelSource;
	}

	_pManageAudioSource.clear();
}


CAudioSource* BridgeSourceClass::Find_Audio_Source(const char *pFileName)
{
	auto it = _pManageAudioSource.find(std::string(pFileName));
	if (it != _pManageAudioSource.end())
	{
		return it->second;
	}

	return 0;
}

bool BridgeSourceClass::Play_Audio_2d(const char* pFileName,const char* pAbsolutePath,bool loop/*=false*/)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Audio_Play2d(loop);
	}
	else
	{
		CAudioSource *pNewSource = new CAudioSource(pAbsolutePath);
		if(pNewSource)
		{
			pNewSource->Audio_Play2d(loop);
			Insert_Audio_Source(pFileName,pNewSource);
			return true;
		}
	}
	return false;
}

bool BridgeSourceClass::Play_Audio_3d(const char* pFileName,const char* pAbsolutePath,float x,float y,float z,bool loop/*=false*/)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Audio_Play3d(x,y,z,loop);
	}
	else
	{
		CAudioSource *pNewSource = new CAudioSource(pAbsolutePath);
		if(pNewSource)
		{
			pNewSource->Audio_Play3d(x,y,z,loop);
			Insert_Audio_Source(pFileName,pNewSource);
			return true;
		}
	}
	return false;
}

bool BridgeSourceClass::Stop_Audio(const char* pFileName)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		pSource->Audio_Stop();
		Erase_Audio_Source(pFileName);
		return true;
	}
	return false;
}

bool BridgeSourceClass::Pause_Audio(const char* pFileName)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Audio_Pause();
	}
	return false;
}

bool BridgeSourceClass::Resume_Audio(const char* pFileName)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Audio_Play();
	}
	return false;
}

bool BridgeSourceClass::Is_Audio_Playing(const char* pFileName)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Is_Audio_Playing();
	}
	return false;
}

bool BridgeSourceClass::Is_Audio_Paused(const char* pFileName)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Is_Audio_Paused();
	}
	return false;
}

bool BridgeSourceClass::Is_Audio_Stopped(const char* pFileName)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Is_Audio_Stopped();
	}
	return false;
}

bool BridgeSourceClass::Is_Loop_Play(const char* pFileName)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Is_Loop_Play();
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Volume(const char* pFileName,float fVolume)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Volume(fVolume);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Volume(const char* pFileName,float& fVolume)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Volume(fVolume);
	}
	return false;
}

bool BridgeSourceClass::Get_Range_Volume(const char* pFileName,float& MinVolume,float& MaxVolume)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Range_Volume(MinVolume,MaxVolume);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Cone_Outer_Volume(const char* pFileName,float fVolume)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Cone_Outer_Volume(fVolume);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Cone_Outer_Volume(const char* pFileName,float &fVolume)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Cone_Outer_Volume(fVolume);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Pitch(const char* pFileName,float fPitch)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Pitch(fPitch);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Pitch(const char* pFileName,float &fPitch)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Pitch(fPitch);
	}
	return false;
}

bool BridgeSourceClass::Audio_Move(const char* pFileName,float x,float y,float z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Audio_Move(x,y,z);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Position(const char* pFileName,float x,float y,float z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Position(x,y,z);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Position(const char* pFileName,float& x,float& y,float& z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Position(x,y,z);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Velocity(const char* pFileName,float x,float y,float z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Velocity(x,y,z);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Velocity(const char* pFileName,float& x,float& y,float& z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Velocity(x,y,z);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Direction(const char* pFileName,int x,int y,int z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Direction(x,y,z);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Direction(const char* pFileName,int& x,int& y,int& z)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Direction(x,y,z);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Rolloff_Factor(const char* pFileName,float factor)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Rolloff_Factor(factor);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Rolloff_Factor(const char* pFileName,float &factor)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Rolloff_Factor(factor);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Reference_Distance(const char* pFileName,float refDis)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Reference_Distance(refDis);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Reference_Distance(const char* pFileName,float &refDis)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Reference_Distance(refDis);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Max_Distance(const char* pFileName,float maxDis)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Max_Distance(maxDis);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Max_Distance(const char* pFileName,float &maxDis)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Max_Distance(maxDis);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Cone_Inner_Angle(const char* pFileName,int nAngle)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Cone_Inner_Angle(nAngle);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Cone_Inner_Angle(const char* pFileName,int &nAngle)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Cone_Inner_Angle(nAngle);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Cone_Outer_Angle(const char* pFileName,int nAngle)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Cone_Outer_Angle(nAngle);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Cone_Outer_Angle(const char* pFileName,int &nAngle)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Cone_Outer_Angle(nAngle);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Doppler_Factor(const char* pFileName,float factor)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Doppler_Factor(factor);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Doppler_Factor(const char* pFileName,float &factor)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Doppler_Factor(factor);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Doppler_Velocity(const char* pFileName,float Vel)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Doppler_Velocity(Vel);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Doppler_Velocity(const char* pFileName,float &Vel)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Doppler_Velocity(Vel);
	}
	return false;
}

bool BridgeSourceClass::Set_Audio_Speed_Of_Sound(const char* pFileName,float speed)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Set_Audio_Speed_Of_Sound(speed);
	}
	return false;
}

bool BridgeSourceClass::Get_Audio_Speed_Of_Sound(const char* pFileName,float &speed)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return pSource->Get_Audio_Speed_Of_Sound(speed);
	}
	return false;
}

void BridgeSourceClass::Set_All_Audio_Pause()
{
	std::unique_lock<std::mutex> lock(_mutex);
	for (const auto &it:_pManageAudioSource)
	{
		CAudioSource *pSource = it.second;
		if(pSource)
		{
			pSource->Audio_Pause();
		}
	}
}

bool BridgeSourceClass::Preload_Audio(const char* pFileName,const char* pAbsolutePath)
{
	std::unique_lock<std::mutex> lock(_mutex);
	CAudioSource *pSource = Find_Audio_Source(pFileName);
	if (pSource)
	{
		return true;
	}
	else
	{
		CAudioSource *pNewSource = new CAudioSource(pAbsolutePath);
		if(pNewSource)
		{
			pNewSource->Preload_Audio_Data();
			Insert_Audio_Source(pFileName,pNewSource);
			return true;
		}
	}
	return false;
}




/******************************************************************************************************************************/

CAudioInterfaceManage* CAudioInterfaceManage::getInstance()
{
	static std::once_flag onceflg;
	static CAudioInterfaceManage *pAudioInterfaceManage = 0;

	std::call_once(onceflg,[](){
		pAudioInterfaceManage = new CAudioInterfaceManage();
	});

	return pAudioInterfaceManage;
}

CAudioInterfaceManage::CAudioInterfaceManage(void)
{
	_pBridgeSourceClass = new BridgeSourceClass();
}


CAudioInterfaceManage::~CAudioInterfaceManage(void)
{
	if(_pBridgeSourceClass)
	{
		delete _pBridgeSourceClass;
		_pBridgeSourceClass = 0;
	}
}

void CAudioInterfaceManage::Destory_All_Audio_Source()
{
	delete this;
}


bool CAudioInterfaceManage::OpenAl_Play_Audio_2d(const char* pFileName,const char* pAbsolutePath,bool loop)
{
	if(!_pBridgeSourceClass)
		return false;
	
	return _pBridgeSourceClass->Play_Audio_2d(pFileName,pAbsolutePath,loop);
}

bool CAudioInterfaceManage::OpenAl_Play_Audio_3d(const char* pFileName,const char* pAbsolutePath,float x,float y,float z,bool loop)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Play_Audio_3d(pFileName,pAbsolutePath,x,y,z,loop);
}

bool CAudioInterfaceManage::OpenAl_Stop_Audio(const char* pFileName)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Stop_Audio(pFileName);
}

bool CAudioInterfaceManage::OpenAl_Pause_Audio(const char* pFileName)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Pause_Audio(pFileName);
}

bool CAudioInterfaceManage::OpenAl_Resume_Audio(const char* pFileName)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Resume_Audio(pFileName);
}

bool CAudioInterfaceManage::OpenAl_Is_Loop_Play(const char* pFileName)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Is_Loop_Play(pFileName);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Volume(const char* pFileName,float fVolume)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Volume(pFileName,fVolume);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Volume(const char* pFileName,float& fVolume)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Volume(pFileName,fVolume);
}

bool CAudioInterfaceManage::OpenAl_Get_Range_Volume(const char* pFileName,float& MinVolume,float& MaxVolume)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Range_Volume(pFileName,MinVolume,MaxVolume);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Cone_Outer_Volume(const char* pFileName,float fVolume)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Cone_Outer_Volume(pFileName,fVolume);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Cone_Outer_Volume(const char* pFileName,float &fVolume)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Cone_Outer_Volume(pFileName,fVolume);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Pitch(const char* pFileName,float fPitch)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Pitch(pFileName,fPitch);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Pitch(const char* pFileName,float &fPitch)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Pitch(pFileName,fPitch);
}

bool CAudioInterfaceManage::OpenAl_Is_Audio_Playing(const char* pFileName)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Is_Audio_Playing(pFileName);
}

bool CAudioInterfaceManage::OpenAl_Is_Audio_Paused(const char* pFileName)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Is_Audio_Paused(pFileName);
}

bool CAudioInterfaceManage::OpenAl_Is_Audio_Stopped(const char* pFileName)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Is_Audio_Stopped(pFileName);
}

bool CAudioInterfaceManage::OpenAl_Audio_Move(const char* pFileName,float x,float y,float z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Audio_Move(pFileName,x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Position(const char* pFileName,float x,float y,float z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Position(pFileName,x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Position(const char* pFileName,float& x,float& y,float& z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Position(pFileName,x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Velocity(const char* pFileName,float x,float y,float z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Velocity(pFileName,x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Velocity(const char* pFileName,float& x,float& y,float& z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Velocity(pFileName,x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Direction(const char* pFileName,int x,int y,int z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Direction(pFileName,x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Direction(const char* pFileName,int& x,int& y,int& z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Direction(pFileName,x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Rolloff_Factor(const char* pFileName,float factor)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Rolloff_Factor(pFileName,factor);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Rolloff_Factor(const char* pFileName,float &factor)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Rolloff_Factor(pFileName,factor);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Reference_Distance(const char* pFileName,float refDis)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Reference_Distance(pFileName,refDis);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Reference_Distance(const char* pFileName,float &refDis)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Reference_Distance(pFileName,refDis);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Max_Distance(const char* pFileName,float maxDis)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Max_Distance(pFileName,maxDis);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Max_Distance(const char* pFileName,float &maxDis)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Max_Distance(pFileName,maxDis);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Cone_Inner_Angle(const char* pFileName,int nAngle)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Cone_Inner_Angle(pFileName,nAngle);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Cone_Inner_Angle(const char* pFileName,int &nAngle)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Cone_Inner_Angle(pFileName,nAngle);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Cone_Outer_Angle(const char* pFileName,int nAngle)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Cone_Outer_Angle(pFileName,nAngle);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Cone_Outer_Angle(const char* pFileName,int &nAngle)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Cone_Outer_Angle(pFileName,nAngle);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Doppler_Factor(const char* pFileName,float factor)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Doppler_Factor(pFileName,factor);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Doppler_Factor(const char* pFileName,float &factor)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Doppler_Factor(pFileName,factor);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Doppler_Velocity(const char* pFileName,float Vel)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Doppler_Velocity(pFileName,Vel);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Doppler_Velocity(const char* pFileName,float &Vel)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Doppler_Velocity(pFileName,Vel);
}

bool CAudioInterfaceManage::OpenAl_Set_Audio_Speed_Of_Sound(const char* pFileName,float speed)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Set_Audio_Speed_Of_Sound(pFileName,speed);
}

bool CAudioInterfaceManage::OpenAl_Get_Audio_Speed_Of_Sound(const char* pFileName,float &speed)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->Get_Audio_Speed_Of_Sound(pFileName,speed);
}

bool CAudioInterfaceManage::OpenAl_Set_All_Audio_Pause()
{
	if(!_pBridgeSourceClass)
		return false;

	_pBridgeSourceClass->Set_All_Audio_Pause();
	return true;
}

bool CAudioInterfaceManage::OpenAl_Preload_Audio(const char* pFileName,const char* pAbsolutePath)
{
	if(!_pBridgeSourceClass)
		return false;

	_pBridgeSourceClass->Preload_Audio(pFileName,pAbsolutePath);
	return true;
}

bool CAudioInterfaceManage::OpenAl_Set_Listener_Direction(float Forward[3],float Up[3])
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->_listener.Set_Listener_Direction(Forward,Up);
}

bool CAudioInterfaceManage::OpenAl_Set_Listener_Position(float x,float y,float z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->_listener.Set_Listener_Position(x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Set_Listener_Velocity(float x,float y,float z)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->_listener.Set_Listener_Velocity(x,y,z);
}

bool CAudioInterfaceManage::OpenAl_Set_Listener_Volume(float fVolume)
{
	if(!_pBridgeSourceClass)
		return false;

	return _pBridgeSourceClass->_listener.Set_Listener_Volume(fVolume);
}


