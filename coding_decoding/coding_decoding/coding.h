//��Ƶ�������    

#include <stdio.h>
#define __STDC_CONSTANT_MACROS     //����꣬����c++

extern "C"
{
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
}


int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index)
{
	int ret;
	int got_frame;
	AVPacket enc_pkt;
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;
	while (1) {
		//printf("Flushing stream #%u encoder\n", stream_index);
		//ret = encode_write_frame(NULL, stream_index, &got_frame);
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_video2 (fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame)
		{ret=0;break;}
		printf("����ɹ�1֡��\n");
		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
	return ret;
}

AVCodecContext* setCodecContext(AVStream* video_st,AVOutputFormat* fmt,int in_w,int in_h)
{
	AVCodecContext* pCodecCtx;    //�����������Ľṹ�壬��������Ƶ����Ƶ������������Ϣ
	//�˴������ñ������
	pCodecCtx = video_st->codec;                     //���ñ���������
	pCodecCtx->codec_id = fmt->video_codec;          //���ñ���������
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;      //�������������������
	
	//���صĸ�ʽ��Ҳ����˵����ʲô����ɫ�ʿռ�������һ�����ص� 
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;            //�������ظ�ʽ
	
	//����Ŀ�����Ƶ֡��С��������Ϊ��λ
	pCodecCtx->width = in_w;                         
	pCodecCtx->height = in_h;       

	//ÿ��25֡
	pCodecCtx->time_base.num = 1;                    
	pCodecCtx->time_base.den = 25;                   

	//ƽ�����ʣ������������ʣ���Ȼ����������Խ����Ƶ��СԽ��
	pCodecCtx->bit_rate = 8000000; 
	//pCodecCtx->rc_max_rate = 400000; //max bit rate
	//pCodecCtx->rc_min_rate = 400000; //min bit rate
	
	//ÿ250֡����1��I֡��I֡Խ�٣���ƵԽС 
	pCodecCtx->gop_size=50;    //�ؼ�֡�������֡��
	pCodecCtx->keyint_min=10;  //�ؼ�֡����С���֡��

	//B��P֡��ǰԤ��ο���֡����ȡֵ��Χ1-16
	//pCodecCtx->refs=1;
	
	//H264
	pCodecCtx->me_range = 16;
	pCodecCtx->max_qdiff = 4;
	 
	//������С����ϵ��  
	pCodecCtx->qmin = 10;     //��С���������ӡ�ȡֵ��Χ1-51��������10-30֮�䡣 
	pCodecCtx->qmax = 51;     //�����������ӡ�ȡֵ��Χ1-51��������10-30֮��
	//��Ϊ���ǵ�����ϵ��q����qmin��qmax֮�両���ģ�  
	//qblur��ʾ���ָ����仯�ı仯�̶ȣ�ȡֵ��Χ0.0��1.0��ȡ0��ʾ������  
	pCodecCtx->qblur = 0.0;  

	//pCodecCtx->qcompress = 0.6;

	//������B֮֡��������ֶ��ٸ�B֡��  
	//����0��ʾ��ʹ��B֡  
	//b ֡Խ�࣬ͼƬԽС  
	//pCodecCtx->max_b_frames = 2;
	
	//b֡�����ɲ��ԣ����Ϊtrue�����Զ�����ʲôʱ����Ҫ����B֡����ߴﵽ���õ����B֡�����������Ϊfalse,��ô����B֡����ʹ�á�
	//pCodecCtx->b_frame_strategy = 0;  

	//�˶�����  
	//pCodecCtx->pre_me = 2;  
  
	//������С������������ճ���  
	//�������ճ��� ��ͳ��ѧ�������˲��ƽ��ֵ��һ�ַ���  
	//pCodecCtx->lmin = 1;  
	//pCodecCtx->lmax = 5;  
    

  
	//�ռ临�Ӷȵ�masking���ȣ�ȡֵ��Χ 0.0-1.0  
	//pCodecCtx->spatial_cplx_masking = 0.0;  
  
	//�˶�����Ԥ�й��ܵ����ȣ���ֵԽ�����ʱ��Խ��  
	//pCodecCtx->me_pre_cmp = 0;  
  
	//���ã�qmin/qmax�ı�ֵ���������ʣ�1��ʾ�ֲ����ô˷�������  
	//pCodecCtx->rc_qsquish = 1;  
  
	//���� i֡��p֡��B֮֡�������ϵ��q�������ӣ����ֵԽ��B֡Խ�����  
	//B֡����ϵ�� = ǰһ��P֡������ϵ��q * b_quant_factor + b_quant_offset  
	//pCodecCtx->b_quant_factor = 1.25;  
  
	//i֡��p֡��B֡������ϵ��������������Խ��B֡Խ�����  
	//pCodecCtx->b_quant_offset = 1.25;  
  
	//p��i������ϵ���������ӣ�Խ�ӽ�1��P֡Խ���  
	//p������ϵ�� = I֡������ϵ�� * i_quant_factor + i_quant_offset  
	//pCodecCtx->i_quant_factor = 0.8;  
	//pCodecCtx->i_quant_offset = 0.0;  
  
	//���ʿ��Ʋ��ʣ��궨�壬��API  
	//pCodecCtx->rc_strategy = 2;  

	//DCT�任�㷨�����ã���7�����ã�����㷨�������Ǹ��ݲ�ͬ��CPUָ����Ż���ȡֵ��Χ��0-7֮��  
	//pCodecCtx->dct_algo = 0;  
  
	//������������ʾ�Թ���������ĳ�����masking�����ȣ�0��ʾ����  
	//pCodecCtx->lumi_masking = 0.0;  
	//pCodecCtx->dark_masking = 0.0;  

	return pCodecCtx;
}



//@ in_filename  �����ļ���
//@ out_filename ����ļ���
//@ in_w         ͼ��Ŀ�
//@ in_h         ͼ��ĸ�	
int coding(const char* in_filename,const char* out_filename,int in_w,int in_h)
{
	AVFormatContext* pFormatCtx;    //��װ��ʽ��������Ϣ��ͳ��ȫ�ֽṹ�壬��������Ƶ�ļ���װ��ʽ�����Ϣ
	AVOutputFormat* fmt;            //���������ʽ

	FILE *in_file = fopen(in_filename, "rb");	//��ƵYUVԴ�ļ� 
	av_register_all();         //ע���������

	//�Ʋ���뷽ʽ
	//����1.���ʹ�ü�������
	//pFormatCtx = avformat_alloc_context();
	//fmt = av_guess_format(NULL, out_filename, NULL);
	//����2
	//�Զ�����һ��AVFormatContext�ڴ棬����������ļ���׺���ж������װ��ʽ��
	avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_filename);  
	fmt = pFormatCtx->oformat; 
	printf("%s\n",fmt->long_name);
	
	//�����ͳ�ʼ��AVIOContext�����Ļ��������ڷ���out_file�ļ�
	//��дģʽʱ��out_fileֻ�ܱ�д����
	//AVIOContext��FFMPEG��������������ݵĽṹ��
	if (avio_open(&pFormatCtx->pb,out_filename, AVIO_FLAG_READ_WRITE) < 0)
	{
		printf("����ļ���ʧ��");
		return -1;
	}

	//�������������AVStream
	AVStream* video_st = avformat_new_stream(pFormatCtx, 0);    
	if (video_st==NULL)
	{
		return -1;
	}

	AVCodecContext* pCodecCtx = setCodecContext(video_st,fmt,in_w,in_h);    //�����������Ľṹ�壬��������Ƶ����Ƶ������������Ϣ
	
	//�����ʽ��Ϣ
	//av_dump_format(pFormatCtx, 0, out_file, 1);     //����ļ���װ����ϸ��Ϣ
	
	//AVCodec* pCodec;              
	//����ƥ��ı�����������У����ر�����
	//ÿ����Ƶ����Ƶ���������(����H.264������)��Ӧһ���ýṹ�� AV_CODEC_ID_H264
	AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	//printf("���������ƣ�%s\n",pCodec->name);
	if (!pCodec)
	{
		printf("û���ҵ����ʵı�������\n");
		return -1;
	}	
	if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0)  //�򿪽�����������0�ɹ�������ʧ��
	{
		printf("��������ʧ�ܣ�\n");
		return -1;
	}

	//�洢һ֡�������������
	AVFrame* picture = av_frame_alloc();      //����AVFrame�ṹ���ڴ棬ʹ��av_frame_free()�ͷ��ڴ�

	//�������ʽ����һ��ͼƬ�Ĵ�С
	int size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	
	//����һ֡���ݵĴ�С���������õĸ�ʽ��Ⱥ͸߶�
	//����ͼƬ���ھ����ͼ������
	uint8_t* picture_buf = (uint8_t *)av_malloc(size);
	avpicture_fill((AVPicture *)picture, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

	//д�ļ�ͷ
	avformat_write_header(pFormatCtx,NULL);

	AVPacket pkt;      //�洢һ֡ѹ���������
	int y_size = pCodecCtx->width * pCodecCtx->height;
	av_new_packet(&pkt,y_size*2);

	int i=0;
	while(fread(picture_buf, 1, y_size*3/2, in_file)){   //����1.5������

		picture->data[0] = picture_buf;          // ����Y
		picture->data[1] = picture_buf+ y_size;  // U 
		picture->data[2] = picture_buf+ y_size*5/4; // V
		
		
		//��ʾʱ���PTS
		picture->pts=i++;

		int got_picture=0;

		//����
		int ret = avcodec_encode_video2(pCodecCtx, &pkt,picture, &got_picture); //����һ֡ѹ�����ݣ������ֽ���
		if(ret < 0)
		{
			printf("�������\n");
			return -1;
		}

		if (got_picture==1)
		{
			printf("����ɹ���%d֡��\n",i-1);
			pkt.stream_index = video_st->index;
			ret = av_write_frame(pFormatCtx, &pkt);    //����������д�뵽�ļ���
			av_free_packet(&pkt);                      //�ͷ�һ����
		}
	}


	//Flush Encoder
	int ret = flush_encoder(pFormatCtx,0);
	if (ret < 0) {
		printf("Flushing encoder failed\n");
		return -1;
	}

	//д�ļ�β
	av_write_trailer(pFormatCtx);

	//����
	if (video_st)
	{
		avcodec_close(video_st->codec);
		av_free(picture);
		av_free(picture_buf);    
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(in_file);

	return 0;
}


