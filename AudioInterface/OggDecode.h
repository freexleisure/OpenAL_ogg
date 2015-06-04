
#ifndef OGGDECODE_H__
#define OGGDECODE_H__




class COggDecode
{
public:
	COggDecode(const char *pFileName);
	~COggDecode();


public:
	int Read_Ogg_File_Data(char* pInputBuf,int nSize);
	bool Set_Ogg_File_Pos(int nPos);
	int get_Ogg_Format();
	int get_Ogg_Frequency();
	long get_File_Size();

private:
	OggVorbis_File _vf;
	ov_callbacks _ov_callbacks;
	int _OggFileAudio;
	vorbis_info *_pVorbis_info;
	vorbis_comment *_pVorbis_comment;

	FILE* _file;
	long _fileSize;

	friend static long tell_func(void *datasource);
	friend static int close_func(void *datasource);
	friend static int seek_func(void *datasource, ogg_int64_t offset, int whence);
	friend static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
};

#endif