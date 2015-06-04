// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#ifndef STDAFX_H__
#define STDAFX_H__



#ifdef _WIN32
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�:
#include <windows.h>
#pragma comment(lib,"../openAL/libs/Win32/OpenAL32.lib")

#ifdef _DEBUG
#pragma comment(lib,"./Debug/libvorbis.lib")
#pragma comment(lib,"./Debug/libogg.lib")
#else
#pragma comment(lib,"./Release/libvorbis.lib")
#pragma comment(lib,"./Release/libogg.lib")
#endif
#include <atltrace.h>

#ifdef _DEBUG
#pragma comment(lib,"atlsd.lib")
#else 
#define ATLTRACE
#endif
#endif//_WINDOWS


#include "../libogg-1.2.2/include/ogg/ogg.h"
#include "../libogg-1.2.2/include/ogg/os_types.h"
#include "../libvorbis-1.3.2/include/vorbis/codec.h"
#include "../libvorbis-1.3.2/include/vorbis/vorbisfile.h"

#include "../openAL/include/al.h"
#include "../openAL/include/alc.h"

#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>

extern ALenum OpenAL_Check_Error();

//#define DATA_STREAM_LOAD   //������ʽ����
#define DATA_ALL_LOAD		//һ���Լ���
#endif