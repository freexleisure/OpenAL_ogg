
#ifndef AUDIO_LISTENER_H__
#define AUDIO_LISTENER_H__

class CListener
{
public:
	CListener(void);
	~CListener(void);

	bool Set_Listener_Direction(ALfloat Forward[3],ALfloat Up[3]);
	bool Set_Listener_Position(ALfloat x,ALfloat y,ALfloat z);
	bool Set_Listener_Velocity(ALfloat x,ALfloat y,ALfloat z);
	bool Set_Listener_Volume(ALfloat fVolume);

private:
	std::mutex _mutex;
};

#endif

