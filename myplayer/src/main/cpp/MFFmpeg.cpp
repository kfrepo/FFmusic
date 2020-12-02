//
// Created by workw on 2019/5/23.
//

#include "MFFmpeg.h"


MFFmpeg::MFFmpeg(PlayStatus *playStatus, FFCallJava *callJava, const char *url) {
    this->playstatus = playStatus;
    this->callJava = callJava;
    this->url = url;
    exit = false;
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
}

MFFmpeg::~MFFmpeg() {

    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
}

int avformat_callback(void *ctx){
    MFFmpeg *fmpeg = (MFFmpeg *) ctx;
    if(fmpeg->playstatus->exit){
        return AVERROR_EOF;
    }
    return 0;
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

    pthread_mutex_lock(&init_mutex);
    av_register_all();
    avformat_network_init();
    pAVFormatCtx = avformat_alloc_context();

    //超时回调
    pAVFormatCtx->interrupt_callback.callback = avformat_callback;
    pAVFormatCtx->interrupt_callback.opaque = this;


    //打开一个文件并解析。可解析的内容包括：视频流、音频流、视频流参数、音频流参数、视频帧索引
    int res = avformat_open_input(&pAVFormatCtx, url, NULL, NULL);
    LOGI("avformat_open_input %s %d", url, res);
    if(res != 0){

        LOGE("can not open url :%s", url);
        callJava->onCallError(CHILD_THREAD, 1001, "can not open url");

        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }


    //查找格式和索引。有些早期格式它的索引并没有放到头当中，需要你到后面探测，就会用到此函数
    if(avformat_find_stream_info(pAVFormatCtx, NULL) < 0){

        LOGE("can not find streams from %s", url);
        callJava->onCallError(CHILD_THREAD, 1002,"can not find streams from url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    LOGI("avformat_find_stream_info numbers %d" , pAVFormatCtx->nb_streams);

    //找出文件中的音频流  nb_streams-视音频流的个数
    for(int i = 0; i < pAVFormatCtx->nb_streams; i++){

        //得到音频流
        if(pAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){

            if(audio == NULL){

                audio = new FFAudio(playstatus, pAVFormatCtx->streams[i]->codecpar->sample_rate, callJava);
                audio->streamIndex = i;
                audio->codecpar = pAVFormatCtx->streams[i]->codecpar;

                audio->duration = pAVFormatCtx->duration / AV_TIME_BASE;
                audio->time_base = pAVFormatCtx->streams[i]->time_base;
                duration = audio->duration;

                //av_q2d(time_base)=每个刻度是多少秒
                LOGI("avformat_find_stream_info i %d, audio duration:%d, audio time_base den:%d, sample_rate:%d" , i, audio->duration, audio->time_base.den, pAVFormatCtx->streams[i]->codecpar->sample_rate);
            }
        }
    }

    //获取解码器 ,FFmpeg的解码器编码器都存在AVCodec的结构体中
    AVCodec *dec = avcodec_find_decoder(audio->codecpar->codec_id);// 软解
//     dec = avcodec_find_decoder_by_name("mp3_mediacodec"); // 硬解
    if(!dec){

        LOGE("can not find decoder");
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    LOGI("audio streamIndex->%d  codecpar-> 编码类型:%d 编码格式:%s" , audio->streamIndex, audio->codecpar->codec_type, dec->name);

    ///配置解码器
    audio->avCodecContext = avcodec_alloc_context3(dec);
    if(!audio->avCodecContext){

        LOGE("can not alloc new decodecctx");
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //将音频流信息拷贝到新的AVCodecContext结构体中 avCodecContext = codecpar
    if(avcodec_parameters_to_context(audio->avCodecContext, audio->codecpar) < 0){
        LOGE("can not fill decodecctx");
        callJava->onCallError(CHILD_THREAD, 1005, "can not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //该函数用于初始化一个视音频编解码器的AVCodecContext,位于libavcodec\avcodec.h 打开解码器
    if(avcodec_open2(audio->avCodecContext, dec, 0) != 0){

        LOGE("cant not open audio strames");
        callJava->onCallError(CHILD_THREAD, 1006, "cant not open audio strames");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (callJava != NULL){

        if (playstatus != NULL && !playstatus->exit){
            callJava->onCallPrepared(CHILD_THREAD);
        } else{
            exit = true;
        }
    }
    pthread_mutex_unlock(&init_mutex);
}

void MFFmpeg::start() {

    if(audio == NULL){
        LOGE("audio is null!");
        callJava->onCallError(CHILD_THREAD, 1006, "audio is null!");
        return;
    }

    audio->play();

    LOGI("audio start decode!");
    int count = 0;
    while(playstatus != NULL && !playstatus->exit){

        if (playstatus->seek){
            av_usleep(1000 * 100);
            continue;
        }
        if (audio->queue->getQueueSize() > 100){
            av_usleep(1000 * 100);
            continue;
        }

        AVPacket *avPacket = av_packet_alloc();
        pthread_mutex_lock(&seek_mutex);
        int ret = av_read_frame(pAVFormatCtx, avPacket);
        pthread_mutex_unlock(&seek_mutex);

        if (ret==0){

//        }
//        if(av_read_frame(pAVFormatCtx, avPacket) == 0){

            if(avPacket->stream_index == audio->streamIndex){

//                LOGI("解码第 %d 帧  DTS:%lld PTS:%lld", count, avPacket->dts, avPacket->pts);
                count++;
                audio->queue->putAVpacket(avPacket);
            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            while(playstatus != NULL && !playstatus->exit){
                if(audio->queue->getQueueSize() > 0){
                    av_usleep(1000 * 100);
                    continue;
                } else{
                    playstatus->exit = true;
                    break;
                }
            }
            break;
        }
    }

    if (callJava != NULL){
        callJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;
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

void MFFmpeg::pause() {
    if(audio != NULL){
        audio->pause();
    }
}

void MFFmpeg::resume() {
    if(audio != NULL){
        audio->resume();
    }
}

void MFFmpeg::release() {

    LOGE("开始释放Ffmpeg");
    if (playstatus->exit){
        return;
    }
    playstatus->exit = true;

    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;

    while(!exit){

        if (sleepCount > 1000){
            exit = true;
        }

        LOGE("wait ffmpeg  exit %d", sleepCount);
        sleepCount++;
        //暂停10毫秒
        av_usleep(1000 * 10);
    }

    LOGE("释放Audio");
    if(audio != NULL){
        audio->release();
        delete(audio);
        audio = NULL;
    }

    LOGE("释放 封装格式上下文");
    if(pAVFormatCtx != NULL){
        avformat_close_input(&pAVFormatCtx);
        avformat_free_context(pAVFormatCtx);
        pAVFormatCtx = NULL;
    }

    LOGE("释放callJava");
    if (callJava != NULL){
        callJava = NULL;
    }

    LOGE("释放 playstatus");
    if(playstatus != NULL){
        playstatus = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
}

void MFFmpeg::seek(int64_t seconds) {

    if (duration <= 0){
        return;
    }
    if (seconds >= 0 && seconds <= duration){
        if (audio != NULL){
            playstatus->seek = true;
            audio->queue->clearAvpacket();
            audio->clock = 0;
            audio->last_tiem = 0;
            pthread_mutex_lock(&seek_mutex);
            int64_t rel = seconds * AV_TIME_BASE;

            //时间基转换公式
            //timestamp(ffmpeg内部时间戳) = AV_TIME_BASE * time(秒)
            //time(秒) = AV_TIME_BASE_Q * timestamp(ffmpeg内部时间戳)
            //要seek的时间点，以time_base或者AV_TIME_BASE为单位
            avformat_seek_file(pAVFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
            pthread_mutex_unlock(&seek_mutex);
            playstatus->seek = false;
        }
    }
}

void MFFmpeg::setVolume(int percent) {
    if (audio != NULL){
        audio->setVolume(percent);
    }

}

void MFFmpeg::setMute(int mute) {
    if (audio != NULL){
        audio->setMute(mute);
    }
}

void MFFmpeg::setPitch(float pitch) {
    if (audio != NULL){
        audio->setPitch(pitch);
    }
}

void MFFmpeg::setSpeed(float speed) {
    if (audio != NULL){
        audio->setSpeed(speed);
    }
}

int MFFmpeg::getSampleRate() {
    if(audio != NULL){
        return audio->avCodecContext->sample_rate;
    }
    return 0;
}

void MFFmpeg::startStopRecord(bool start) {
    if(audio != NULL){
        audio->startStopRecord(start);
    }
}

/**
AVPacket：存储压缩数据（视频对应H.264等码流数据，音频对应AAC/MP3等码流数据）
AVFrame：存储非压缩的数据（视频对应RGB/YUV像素数据，音频对应PCM采样数据）
**/