// coding_decoding.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "coding.h"
#include "decoding.h"

int _tmain(int argc, _TCHAR* argv[])
{
	const char *in_file_codec = "Friends.mkv";

	const char *out_file_decodec = "Friends.YUV";				//����ļ�
	//decoding(in_file_codec,out_file_decodec);

	
	int in_w=1280;
	int in_h=720;
	
	const char *out_file_codec = "Friends264.h264";				//����ļ�
	coding(out_file_decodec,out_file_codec,in_w,in_h);

	return 0;
}

