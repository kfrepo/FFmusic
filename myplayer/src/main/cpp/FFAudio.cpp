//
// Created by workw on 2019/5/23.
//

#include "FFAudio.h"


FFAudio::FFAudio(PlayStatus *playStatus, int sample_rate, FFCallJava *callJava) {

    this->playstatus = playStatus;
    this->callJava = callJava;
    this->sample_rate = sample_rate;
    queue = new AVPacketQueue(playstatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);
    pthread_mutex_init(&codecMutex, NULL);

    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);
};

FFAudio::~FFAudio() {
    pthread_mutex_destroy(&codecMutex);
};

void *decodPlay(void *data){

    FFAudio *ffAudio = (FFAudio *) data;
    LOGI("START initOpenSLES!");
    ffAudio->initOpenSLES();
//    pthread_exit(&ffAudio->thread_play);
    return 0;
}

void *pcmCallBack(void *data){
    FFAudio *audio = static_cast<FFAudio *>(data);
    while (audio->playstatus!=NULL && !audio->playstatus->exit){
        PcmBean *pcmBean = NULL;
        audio->bufferQueue->getBuffer(&pcmBean);
        if (pcmBean == NULL){
            continue;
        }
//        LOGI("pcmCallBack pcmbean buffer size is %d", pcmBean->buffsize);

        if (pcmBean->buffsize <= audio->defaultPcmSize){
            if(audio->isRecordPcm){
                audio->callJava->onCallPcmToAAc(CHILD_THREAD, pcmBean->buffsize, pcmBean->buffer);
            }
        } else{

            //分包
            int pack_num = pcmBean->buffsize / audio->defaultPcmSize;
            int pack_sub = pcmBean->buffsize % audio->defaultPcmSize;

//            LOGI("pcmbean 分包 pack_num %d pack_sub %d", pack_num, pack_sub);
            for (int i = 0; i < pack_num; ++i) {
                char *bf = static_cast<char *>(malloc(audio->defaultPcmSize));
                memcpy(bf, pcmBean->buffer + i*audio->defaultPcmSize, audio->defaultPcmSize);
                if(audio->isRecordPcm){
                    audio->callJava->onCallPcmToAAc(CHILD_THREAD, audio->defaultPcmSize, bf);
                }
                free(bf);
                bf = NULL;
            }

            if (pack_sub > 0){
                char *bf = static_cast<char *>(malloc(pack_sub));
                memcpy(bf, pcmBean->buffer + pack_num * audio->defaultPcmSize, pack_sub);
                if(audio->isRecordPcm){
                    audio->callJava->onCallPcmToAAc(CHILD_THREAD, pack_sub, bf);
                }
                free(bf);
                bf = NULL;
            }
        }
        delete(pcmBean);
        pcmBean = NULL;
    }
    pthread_exit(&audio->pcmCallBackThread);
}

void FFAudio::play(){
    if (playstatus != NULL && !playstatus->exit){
        bufferQueue = new FFBufferQueue(playstatus);
        pthread_create(&thread_play, NULL, decodPlay, this);
        pthread_create(&pcmCallBackThread, NULL, pcmCallBack, this);
    }
}


//音频重采样为PCM
//FILE *outFile = fopen("/mnt/sdcard/Music/resamplemymusic.pcm", "w");
int FFAudio::resampleAudio(void **pcmbuf) {

    //LOGD("START resampleAudio!");
    data_size = 0;
    while (playstatus != NULL && !playstatus->exit){

        if (playstatus->seek){
            av_usleep(1000 * 100);
            continue;
        }
//        LOGI("FFAudio queue->getQueueSize() %d", queue->getQueueSize());
        if(queue->getQueueSize() == 0){
            if (!playstatus->load){
                playstatus->load = true;
                callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else{
            if (playstatus->load){
                playstatus->load = false;
                callJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        avPacket = av_packet_alloc();
        if (queue->popAVpacket(avPacket) != 0){

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        pthread_mutex_lock(&codecMutex);
        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&codecMutex);
            continue;
        }

        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0){
            if (avFrame->channels > 0 && avFrame->channel_layout == 0){

                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0){

                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }
            //LOGI("解码 audio AVframe  %d", avFrame->pkt_size);

            SwrContext *swr_ctx = NULL;
            //根据通道布局、音频数据格式、采样频率，返回分配的转换上下文
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    static_cast<AVSampleFormat> (avFrame->format),
                    avFrame->sample_rate,
                    NULL, NULL);

            if (!swr_ctx || swr_init(swr_ctx) < 0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;

                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swr_ctx);
                pthread_mutex_unlock(&codecMutex);
                continue;
            }

            /**
             * 针对每一帧音频的处理。把一帧帧的音频作相应的重采样
             * 参数1：音频重采样的上下文 参数
             * 参数2：输出的指针。传递的输出的数组 参数
             * 参数3：输出的样本数量，不是字节数。单通道的样本数量。
             * 参数4：输入的数组，AVFrame解码出来的DATA
             * 参数5：输入的单通道的样本数量。
             */
            nb = swr_convert(
                    swr_ctx,
                    &buffer,
                    avFrame->nb_samples,
                    (const uint8_t **) avFrame->data,
                    avFrame->nb_samples);
//            LOGI("swr_convert = %d", nb);

            //根据通道的layout返回通道的个数
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            //data_size 元素的个数
            //fwrite(buffer, 1, data_size, outFile);
            //LOGD("nb is %d, out_channels is %d, data_size is %d", nb, out_channels, data_size);

            //av_q2d(time_base)=每个刻度是多少秒  pts*av_q2d(time_base)才是帧的显示时间戳
            now_time = avFrame->pts * av_q2d(time_base);
            //LOGD("now_time:%lf, time_base:%d/%d, pts:%lld", now_time, time_base.num, time_base.den, avFrame->pts);
            if(now_time < clock){
                now_time = clock;
            }
            clock = now_time;
            *pcmbuf = buffer;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;

            swr_free(&swr_ctx);
            swr_ctx = NULL;
            pthread_mutex_unlock(&codecMutex);
            break;
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            pthread_mutex_unlock(&codecMutex);
            continue;
        }
    }
    //fclose(outFile);
    //LOGD("Success resampleAudio!");
    return data_size;
}


int FFAudio::getSoundTouchData() {
    while (playstatus!=NULL && !playstatus->exit){
        out_buffer = NULL;
        if (finished){
            finished = false;
            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
            if (data_size > 0){
                for (int i = 0; i < data_size/2; ++i) {
                    sampleBuffer[i] = (out_buffer[i*2]) | ((out_buffer[i*2 + 1]) << 8);
                }
                //将8bit的数据转换成16bit 后再给SoundTouch处理
                //把PCM数据给SoundTouch处理
                soundTouch->putSamples(sampleBuffer, nb);
                //循环得到处理后的PCM数据
                num = soundTouch->receiveSamples(sampleBuffer, data_size/4);
            } else{
                soundTouch->flush();
            }
        }

        if(num == 0){
            finished = true;
            continue;
        } else{
            if(out_buffer == NULL){
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if(num == 0){
                    finished = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}


void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context) {

    FFAudio *ffAudio = (FFAudio *) context;
    if(ffAudio != NULL) {

        int buffersize = ffAudio->getSoundTouchData();
        //LOGD("pcmBufferCallBack buffersize %d", buffersize);

        if(buffersize > 0) {

            //假设某通道的音频信号是采样率为8kHz，位宽为16bit，20ms一帧，双通道，则一帧音频数据的大小为： int size = 8000 x 16bit x 0.02s x 2 = 5120 bit = 640 byte
            ffAudio->clock += buffersize / ((double) (ffAudio->sample_rate * 2 * 2));

            //LOGI("FFAudio pcmBufferCallBack clock is %lf, last_time is %lf", ffAudio->clock, ffAudio->last_time);
            if(ffAudio->clock - ffAudio->last_time >= 0.1) {

                ffAudio->last_time = ffAudio->clock;
                ffAudio->callJava->onCallTimeInfo(CHILD_THREAD, ffAudio->clock, ffAudio->duration);
            }

            //边播边录
            if (ffAudio->isRecordPcm){
                //ffAudio->callJava->onCallPcmToAAc(CHILD_THREAD, buffersize * 2 * 2, ffAudio->sampleBuffer);
                ffAudio->bufferQueue->putBuffer(ffAudio->sampleBuffer, buffersize * 2 * 2);
            }

            //音量
            ffAudio->callJava->onCallValumeDB(CHILD_THREAD,
                                              ffAudio->getPCMDB(reinterpret_cast<char *>(ffAudio->sampleBuffer),
                                                      buffersize * 4));

            // 调用BufferQueue的Enqueue方法，把输入数据取到buffer
            (* ffAudio-> pcmBufferQueue)->Enqueue(ffAudio->pcmBufferQueue, (char *) ffAudio->sampleBuffer, buffersize*2*2);
        }
    }
}


void FFAudio::initOpenSLES() {

    SLresult result;
    LOGI("第一步 opensl 创建并初始化Audio Engine");
    //第一步， 创建并初始化Audio Engine
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    // 初始化上一步得到的engineObject
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    // 获取SLEngine接口对象，后续的操作将使用这个对象
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步 创建混音器
    const SLInterfaceID mids[2] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[2] = {SL_BOOLEAN_FALSE};

    // 使用第一步的engineEngine，创建音频输出Output Mix
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    // 输出管道
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};

    // 第三步，配置PCM格式信息
    // Buffer Queue的参数
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    // 设置音频格式
    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            static_cast<SLuint32>(getCurrentSampleRateForOpensles(sample_rate)),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    // 输出源
    SLDataSource slDataSource = {&android_queue, &pcm};

    LOGI("第四步 opensl 创建播放器");
    //第四步，创建播放器
    const SLInterfaceID ids[4] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE, SL_IID_MUTESOLO};
    const SLboolean req[4] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    // 创建音频播放对象AudioPlayer
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 4, ids, req);

    //初始化AudioPlayer
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    // 得到接口后调用，获取播放器接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

    // 获取声音接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);

    // 获取声道接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);

    // 注册回调缓冲区，获取缓冲队列接口，即音频输出的BufferQueue接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);

    //输出缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);

    LOGI("opensl 播放器 开始播放!");
    // 获取播放状态接口,设置为播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);
    setVolume(1);
}


int FFAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate)
    {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate =  SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void FFAudio::pause() {
    if(pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

void FFAudio::resume() {
    if(pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_PLAYING);
    }
}

void FFAudio::stop() {
    if(pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}

void FFAudio::release() {
    LOGE("开始释放FFAudio!");
    if (bufferQueue != NULL){
        bufferQueue->noticeThread();

        bufferQueue->release();
        delete(bufferQueue);
        bufferQueue = NULL;
    }
//    pthread_join(pcmCallBackThread, NULL);
    pthread_join(thread_play, NULL);

    if (queue != NULL){
        delete(queue);
        queue = NULL;
    }

    if (pcmPlayerObject != NULL) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
        pcmMutePlay = NULL;
        pcmVolumePlay = NULL;
    }

    if(outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    if(engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    if(buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    if(out_buffer != NULL){
        //同buffer 不用free
        out_buffer = NULL;
    }
    if(soundTouch == NULL){
        delete soundTouch;
        soundTouch = NULL;
    }
    if(sampleBuffer != NULL){
        free(sampleBuffer);
        sampleBuffer = NULL;
    }

    if(avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }

    if(playstatus != NULL) {
        playstatus = NULL;
    }

    if(callJava != NULL) {
        callJava = NULL;
    }
    LOGE("释放FFAudio成功!");
}

void FFAudio::setVolume(int percent) {

    LOGI("设置音量 %d", percent);
    if(pcmVolumePlay != NULL){
        if(percent > 30){
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        }else if(percent > 25){
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        }else if(percent > 20){
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        }else if(percent > 15){
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        }else if(percent > 10){
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        }else if(percent > 5){
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        }else if(percent > 3){
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        }else if(percent > 0){
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        }else{
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void FFAudio::setMute(int mute) {
    LOGI("音频声道 %d", mute);
    if (pcmMutePlay != NULL){

        if (mute == 0){
            //右
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, true);
        } else if(mute == 1) {
            //左
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, true);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        } else if (mute == 2){
            //立体
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        }
    }
}

void FFAudio::setPitch(float pitch) {
    LOGI("音频变调 %f", pitch);
    this->pitch = pitch;
    if (soundTouch != NULL){
        soundTouch->setPitch(pitch);
    }
}

void FFAudio::setSpeed(float speed) {
    LOGI("音频变速 %f", speed);
    this->speed = speed;
    if (soundTouch != NULL){
        soundTouch->setTempo(speed);
    }
}

/**
 * 获取所有振幅之平均值 计算db (振幅最大值 2^16-1 = 65535 最大值是 96.32db)
 * 16 bit == 2字节 == short int
 * 无符号16bit：96.32=20*lg(65535);
 *
 * @param pcmdata 转换成char类型，才可以按字节操作
 * @param size pcmdata的大小
 * @return
 */
int FFAudio::getPCMDB(char *pcmcata, size_t pcmsize) {
    int db = 0;
    short int pervalue = 0;
    double sum = 0;
    for(int i = 0; i < pcmsize; i+= 2){
        memcpy(&pervalue, pcmcata+i, 2);
        sum += abs(pervalue);
    }
    sum = sum / (pcmsize / 2);
    if(sum > 0){
        db = (int)20.0 *log10(sum);
    }
    return db;
}

void FFAudio::startStopRecord(bool b) {
    this->isRecordPcm = b;
}
