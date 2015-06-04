#include "stdafx.h"
#include <stdio.h>


ALenum OpenAL_Check_Error()
{
	ALenum nError = alGetError(); 
	if (nError)
	{
		//fprintf(stderr, "OpenAL Check Error %d\n",nError);
		//fprintf(stdout, "OpenAL Check Error %d\n",nError);
		
#ifdef _WIN32
		ATLTRACE("-------OpenAL Check Error code:%d  %s------\n",nError,alGetString(nError));
#else
		fprintf(stderr, "-------OpenAL Check Error code:%d  %s------\n",nError,alGetString(nError));
#endif
	}
	return nError;
}
