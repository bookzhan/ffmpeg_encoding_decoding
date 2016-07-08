// ffmpegdecodec.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#define __STDC_CONSTANT_MACROS     //����꣬����c++
extern "C"
{
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
}
int decode_write_frame(FILE *fp_YUV, AVCodecContext *pCodecCtx,struct SwsContext *img_convert_ctx,
                    AVFrame *pFrameYUV,AVFrame *pFrame,int frame_cnt, AVPacket *packet, int size_picture)
{
	int got_picture;
	int	ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
	if (ret < 0) {
		printf("Decode Error.(�������)\n");
		return ret;
	}
	if (!got_picture)
		return 0;
	if (got_picture) {
		printf("Flush Decoder: Succeed to decode %d frame!\n",frame_cnt);
		sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
			pFrameYUV->data, pFrameYUV->linesize);

		fwrite(pFrameYUV->data[0],1,size_picture,fp_YUV);     //Y
		fwrite(pFrameYUV->data[1],1,size_picture/4,fp_YUV);   //U
		fwrite(pFrameYUV->data[2],1,size_picture/4,fp_YUV);   //V
	}
	//av_free_packet(packet);
	return got_picture;
}

//@ in_filename �����ļ���
//@ out_filename ����ļ���
int decoding(const char *in_filename,const char *out_filename)
{	
	av_register_all();           //ע���������
	avformat_network_init();   //��ʼ������ģ�飬���Ǳ����

	//��װ��ʽ��������Ϣ��ͳ��ȫ�ֽṹ�壬��������Ƶ�ļ���װ��ʽ�����Ϣ��
	AVFormatContext * pFormatCtx = avformat_alloc_context(); //����һ��AVFormatContext�ṹ���ڴ棬ʹ��avformat_free_context()�ͷ��ڴ�
	if(avformat_open_input(&pFormatCtx,in_filename,NULL,NULL)!=0){  //���������ļ������ʹ��avformat_close_input()�رգ�����0�ɹ�
		printf("Couldn't open input stream.\n");      
		return -1;
	}
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){  //��ȡmedia�ļ�����ȡ���ļ�������û��ͷ�ļ����ļ���ʽ���ɹ����ش���0
		printf("Couldn't find stream information.\n");
		return -1;
	}

	//�˴����ж��ǲ�����Ƶ�ļ�0,1�ֱ��ʾ��Ƶ����Ƶ������
	int videoindex=-1;
	//printf("%d\n",pFormatCtx->nb_streams);

	for(int i=0; i<pFormatCtx->nb_streams; i++) 
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	//printf("%d\n",videoindex);      //�����Ƶ�����

	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}

	//����ƥ��Ľ�����������У����ؽ�����
	//�����������Ľṹ�壬��������Ƶ����Ƶ������������Ϣ��
	AVCodecContext *pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	
	//ÿ����Ƶ����Ƶ���������(����H.264������)��Ӧһ���ýṹ�塣
	AVCodec *pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	//pCodec=avcodec_find_decoder_by_name("h264");   //���ս��������Ʋ���
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}
	printf("%s\n",pCodec->name);      //�������������

	//�򿪽�����������0�ɹ�������ʧ��
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}
	
	/*
	 * �ڴ˴���������Ƶ��Ϣ�Ĵ���
	 * ȡ����pFormatCtx��ʹ��fprintf()
	 */
	FILE *fp=fopen("xx.txt","wb+");
	fprintf(fp,"��Ƶʱ����%d\n",pFormatCtx->duration);                //�����Ƶ���ȣ���λ��΢��
	fprintf(fp,"��װ��ʽ��%s\n",pFormatCtx->iformat->long_name);      //�����Ƶ��װ��ʽ
	fprintf(fp,"�ֱ��ʣ�%d*%d\n",pFormatCtx->streams[videoindex]->codec->width,pFormatCtx->streams[videoindex]->codec->height);      //�����Ƶ�ķֱ���
	fprintf(fp,"֡�ʣ�%d\n",pFormatCtx->streams[videoindex]->r_frame_rate);      //�����Ƶ�ķֱ���
	fclose(fp);

	//�洢һ֡�������������
	AVFrame *pFrame=av_frame_alloc();       //����AVFrame�ṹ���ڴ棬ʹ��av_frame_free()�ͷ��ڴ�
	AVFrame *pFrameYUV=av_frame_alloc();

	//����һ֡���ݵĴ�С���������õĸ�ʽ��Ⱥ͸߶�
	//����ͼƬ���ھ����ͼ������
	uint8_t *out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));	
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	
	//���ٴ洢һ֡ѹ���������ݵ��ڴ�
	//�洢һ֡ѹ����������
	AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	////Output Info-----------------------------
	//printf("--------------- File Information ----------------\n");
	//av_dump_format(pFormatCtx,0,in_filename,0);                       //��������ļ���װ����ϸ��Ϣ
	//printf("-------------------------------------------------\n");
	
	//���ٺͷ���һ��SwsContext����ʼ��һ��SwsContext�����������libswscale��Դ���룬����ͼ����ʹ��sws_freeContext()�ͷ�
	//���ڴ���ͼƬ�������ݵ���⡣�������ͼƬ���ظ�ʽ��ת����ͼƬ������ȹ���
	struct SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 

	int frame_cnt=0;
	int size_picture=pCodecCtx->width*pCodecCtx->height;
	FILE *fp_YUV=fopen(out_filename,"wb+");

	while(av_read_frame(pFormatCtx, packet)>=0){      //�������ļ�����һ֡ѹ���������ݣ�����0��ȷ
		if(packet->stream_index==videoindex){
		/*
			* �ڴ˴�������H264�����Ĵ���
			* ȡ����packet��ʹ��fwrite()
			*/
			decode_write_frame(fp_YUV, pCodecCtx,img_convert_ctx,
							 pFrameYUV,pFrame,frame_cnt,packet,size_picture);    //���벢д����
			frame_cnt++;
		}
		av_free_packet(packet);
	}

	//Flush Decoder
	packet->data = NULL;
    packet->size = 0;
	int flag=1;
	while(flag){
		flag=decode_write_frame(fp_YUV, pCodecCtx,img_convert_ctx,
							 pFrameYUV,pFrame,frame_cnt,packet,size_picture);
		av_free_packet(packet);
	}

	fclose(fp_YUV);    //�ر��ļ�

	//free
	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrame);          
	av_frame_free(&pFrameYUV);
	avcodec_close(pCodecCtx);      //�رս�����

	avformat_free_context(pFormatCtx);
 	return 0;
}


