//
// Created by workw on 2019/5/23.
//

#include "MFFmpeg.h"
#include "AndroidLog.h"

MFFmpeg::MFFmpeg(PlayStatus *playStatus, FFCallJava *callJava, const char *url) {
    this->playstatus = playStatus;
    this->callJava = callJava;
    this->url = url;
}

void *decodeFFmpeg(void *data){

    MFFmpeg *mfFmpeg = (MFFmpeg *) data;
    mfFmpeg->decodeFFmpegThread();
    pthread_exit(&mfFmpeg->decodeThread);
}

void MFFmpeg::parpared() {

    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

void MFFmpeg::decodeFFmpegThread() {


    av_register_all();
    avformat_network_init();
    pAVFormatCtx = avformat_alloc_context();
    LOGI("avformat_open_input %s", url);
    //打开一个文件并解析。可解析的内容包括：视频流、音频流、视频流参数、音频流参数、视频帧索引
    if(avformat_open_input(&pAVFormatCtx, url, NULL, NULL) != 0){

        if(LOG_DEBUG){
            LOGE("can not open url :%s", url);
        }
        return;
    }


    //查找格式和索引。有些早期格式它的索引并没有放到头当中，需要你到后面探测，就会用到此函数
    if(avformat_find_stream_info(pAVFormatCtx, NULL) < 0){
        if(LOG_DEBUG){
            LOGE("can not find streams from %s", url);
        }
        return;
    }

    LOGI("avformat_find_stream_info %d" , pAVFormatCtx->nb_streams);
    //找出文件中的音频流  nb_streams-视音频流的个数
    for(int i = 0; i < pAVFormatCtx->nb_streams; i++){
        if(pAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){

            if(audio == NULL){

                audio = new FFAudio(playstatus, pAVFormatCtx->streams[i]->codecpar->sample_rate);
                audio->streamIndex = i;
                audio->codecpar = pAVFormatCtx->streams[i]->codecpar;
            }
        }
    }

    //获取解码器 ,FFmpeg的解码器编码器都存在AVCodec的结构体中
    AVCodec *dec = avcodec_find_decoder(audio->codecpar->codec_id);// 软解
//     dec = avcodec_find_decoder_by_name("mp3_mediacodec"); // 硬解
    if(!dec){
        if(LOG_DEBUG){
            LOGE("can not find decoder");
        }
        return;
    }

    LOGI("audio streamIndex->%d  codecpar-> 编码类型:%d 编码格式:%s" , audio->streamIndex, audio->codecpar->codec_type, dec->name);

    ///配置解码器
    audio->avCodecContext = avcodec_alloc_context3(dec);
    if(!audio->avCodecContext){
        if(LOG_DEBUG){
            LOGE("can not alloc new decodecctx");
        }
        return;
    }

    //将音频流信息拷贝到新的AVCodecContext结构体中 avCodecContext = codecpar
    if(avcodec_parameters_to_context(audio->avCodecContext, audio->codecpar) < 0){
        if(LOG_DEBUG){
            LOGE("can not fill decodecctx");
        }
        return;
    }

    //该函数用于初始化一个视音频编解码器的AVCodecContext,位于libavcodec\avcodec.h
    if(avcodec_open2(audio->avCodecContext, dec, 0) != 0){
        if(LOG_DEBUG){
            LOGE("cant not open audio strames");
        }
        return;
    }

    callJava->onCallPrepared(CHILD_THREAD);
}

void MFFmpeg::start() {

    if(audio == NULL){
        if(LOG_DEBUG){
            LOGE("audio is null");
            return;
        }
    }


    audio->play();

    LOGI("audio start decode!");
    int count = 0;
    while(playstatus != NULL && !playstatus->exit)
    {
        AVPacket *avPacket = av_packet_alloc();
        if(av_read_frame(pAVFormatCtx, avPacket) == 0)
        {
            if(avPacket->stream_index == audio->streamIndex)
            {
                //解码操作
                count++;
//                LOGI("解码第 %d 帧  DTS:%lld PTS:%lld", count, avPacket->dts, avPacket->pts);

                audio->queue->putAVpacket(avPacket);
            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else{

            av_packet_free(&avPacket);
            av_free(avPacket);
            while(playstatus != NULL && !playstatus->exit)
            {

                if(audio->queue->getQueueSize() > 0)
                {
                    continue;
                } else{
                    playstatus->exit = true;
                    break;
                }
            }
        }
    }

    LOGE("解码完成");
//    while(1)
//    {
//        //分配一个结构体大小的内存,返回的是一个AVPacket的一个指针
//        AVPacket *avPacket = av_packet_alloc();
//
//        //读取具体的音/视频帧数据
//        if(av_read_frame(pAVFormatCtx, avPacket) == 0)
//        {
//            //stream_index：标识该AVPacket所属的视频/音频流
//            if(avPacket->stream_index == audio->streamIndex){
//                //解码操作
//                count++;
//                LOGI("解码第 %d 帧  DTS:%lld PTS:%lld", count, avPacket->dts, avPacket->pts);
//                audio->queue->putAVpacket(avPacket);
//
//            } else{
//                av_packet_free(&avPacket);
//                av_free(avPacket);
//            }
//        } else{
//
//            LOGE("decode finished");
//
//            av_packet_free(&avPacket);
//            av_free(avPacket);
//            break;
//        }
//    }

//    //模拟出队
//    while (audio->queue->getQueueSize() > 0){
//
//        AVPacket *packet = av_packet_alloc();
//        audio->queue->popAVpacket(packet);
//        av_packet_free(&packet);
//        av_free(packet);
//        packet = NULL;
//    }



}

/**
AVPacket：存储压缩数据（视频对应H.264等码流数据，音频对应AAC/MP3等码流数据）
AVFrame：存储非压缩的数据（视频对应RGB/YUV像素数据，音频对应PCM采样数据）
**/