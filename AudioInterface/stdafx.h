// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#ifndef STDAFX_H__
#define STDAFX_H__



#ifdef _WIN32
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
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

//#define DATA_STREAM_LOAD   //数据流式加载
#define DATA_ALL_LOAD		//一次性加载
#endif