
#ifndef AUDIO_DEVICE_H__
#define AUDIO_DEVICE_H__


class CAudioDevice
{
public:
	CAudioDevice(void);
	~CAudioDevice(void);

	bool Create_OpenAL_Device();
	void Destory_OpenAL_Device();

private:
	CAudioDevice(const CAudioDevice&);
	CAudioDevice& operator=(const CAudioDevice&);

protected:
	ALCdevice *_pAlcDevice;
};


#endif
