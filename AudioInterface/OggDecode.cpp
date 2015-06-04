#include "stdafx.h"
#include "OggDecode.h"


union bigendianp
{
	short n;
	char c;
}SysMemory={1};

static int Sys_Memory_Type()
{
	if(SysMemory.c == 1)
		return 0;

	return 1;
}


static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	COggDecode *pOgg = (COggDecode*)datasource;
	if(pOgg)
	{
		return fread(ptr,sizeof(char),size*nmemb,pOgg->_file);
	}
	return 0;
}

static int seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	COggDecode *pOgg = (COggDecode*)datasource;
	switch (whence)
	{
	case SEEK_SET:
		fseek(pOgg->_file, offset, SEEK_SET);
		break;
	case SEEK_CUR:
		fseek(pOgg->_file,offset,SEEK_CUR);
		break;
	case SEEK_END:
		fseek(pOgg->_file,pOgg->_fileSize-offset,SEEK_SET);
		break;
	}
	return 0;
}

static int close_func(void *datasource)
{
	return 0;
}

static long tell_func(void *datasource)
{
	COggDecode *pOgg = (COggDecode*)datasource;
	return ftell(pOgg->_file);
}

/******************************************************************************************************************************************/


COggDecode::COggDecode(const char *pFileName)
	:_file(0),
	_OggFileAudio(-1)
{

	if(pFileName && pFileName[0] != 0)
	{
		_file = fopen(pFileName,"rb");
		if (_file)
		{
			fseek(_file,0,SEEK_END);
			_fileSize = ftell(_file);
			fseek(_file,0,SEEK_SET);
		}
	}

	_ov_callbacks.read_func  = read_func;
	_ov_callbacks.close_func = close_func;
	_ov_callbacks.seek_func  = seek_func;
	_ov_callbacks.tell_func  = tell_func;

	_OggFileAudio = ov_open_callbacks(this,&_vf,0,0,_ov_callbacks);
	if(_OggFileAudio == 0)
	{
		_pVorbis_info    = ov_info(&_vf,-1);
		_pVorbis_comment = ov_comment(&_vf,-1);
	}
}


COggDecode::~COggDecode()
{
	ov_clear(&_vf);
	if (_file)
	{
		fclose(_file);
	}
}

int COggDecode::Read_Ogg_File_Data(char* pInputBuf,int nSize)
{
	if (_OggFileAudio == 0)
	{
		int bitstream = 0;
		return ov_read(&_vf,pInputBuf,nSize,Sys_Memory_Type(),2,1,&bitstream);
	}
	return 0;
}

bool COggDecode::Set_Ogg_File_Pos(int nPos)
{
	if (_OggFileAudio == 0)
	{
		if(ov_seekable(&_vf))
		{
			return ov_raw_seek(&_vf,nPos) == 0;
		}
	}
	return false;
}

int COggDecode::get_Ogg_Frequency()
{
	if(_OggFileAudio == 0)
	{
		return _pVorbis_info->rate;
	}
	return 0;
}

int COggDecode::get_Ogg_Format()
{
	if(_OggFileAudio == 0)
	{
		if( _pVorbis_info->channels == 1)
			return AL_FORMAT_MONO16;
		else 
			return AL_FORMAT_STEREO16;
	}
	return AL_FORMAT_MONO8;
}

long COggDecode::get_File_Size()
{
	//return _fileSize;
	//return ov_pcm_total(&_vf,-1)*_pVorbis_info->channels*2;
	return ov_pcm_total(&_vf,-1)*_pVorbis_info->channels;
}
