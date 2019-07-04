//
// Created by workw on 2019/5/23.
//

#include "FFAudio.h"

FFAudio::FFAudio(PlayStatus *playStatus, int sample_rate) {

    this->playstatus = playStatus;

    this->sample_rate = sample_rate;
    queue = new AVPacketQueue(playstatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);
};

FFAudio::~FFAudio() {

};

void *decodPlay(void *data){

    FFAudio *ffAudio = (FFAudio *) data;
    LOGI("START initOpenSLES!");
    ffAudio->initOpenSLES();
    pthread_exit(&ffAudio->thread_play);
}

void FFAudio::play(){
    pthread_create(&thread_play, NULL, decodPlay, this);
}


//音频重采样为PCM
//FILE *outFile = fopen("/mnt/sdcard/Music/resamplemymusic.pcm", "w");
int FFAudio::resampleAudio() {

    LOGI("START resampleAudio!");
    while (playstatus != NULL && !playstatus->exit)
    {
        avPacket = av_packet_alloc();
        if (queue->popAVpacket(avPacket) != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0)
        {
            if (avFrame->channels && avFrame->channel_layout == 0)
            {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0)
            {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swr_ctx;
            //根据通道布局、音频数据格式、采样频率，返回分配的转换上下文
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    NULL, NULL
                    );

            if (!swr_ctx || swr_init(swr_ctx) < 0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swr_ctx);
                continue;
            }

            /**
             * 针对每一帧音频的处理。把一帧帧的音频作相应的重采样
             * 参数1：音频重采样的上下文 参数2：输出的指针。传递的输出的数组 参数3：输出的样本数量，不是字节数。单通道的样本数量。 参数4：输入的数组，AVFrame解码出来的DATA 参数5：输入的单通道的样本数量。
             */
            int nb = swr_convert(
                    swr_ctx,
                    &buffer,
                    avFrame->nb_samples,
                    (const uint8_t **) avFrame->data,
                    avFrame->nb_samples
                    );
//            LOGI("swr_convert = %d", nb);

            //根据通道的layout返回通道的个数
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            //data_size 元素的个数
            //fwrite(buffer, 1, data_size, outFile);
            LOGI("nb is %d, out_channels is %d, data_size is %d", nb, out_channels, data_size);

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;

            swr_free(&swr_ctx);
            break;
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
    //fclose(outFile);
    LOGI("Success resampleAudio!");
    return data_size;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context)
{
    FFAudio *ffAudio = (FFAudio *) context;
    if(ffAudio != NULL)
    {
        int buffersize = ffAudio->resampleAudio();
        LOGI("pcmBufferCallBack %d", buffersize);
        if(buffersize > 0)
        {
            // 调用BufferQueue的Enqueue方法，把输入数据取到buffer
            (* ffAudio-> pcmBufferQueue)->Enqueue( ffAudio->pcmBufferQueue, (char *) ffAudio-> buffer, buffersize);
        }
    }
}


void FFAudio::initOpenSLES() {

    SLresult result;
    LOGI("创建并初始化Audio Engine");
    //第一步， 创建并初始化Audio Engine
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    // 初始化上一步得到的engineObject
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    // 获取SLEngine接口对象，后续的操作将使用这个对象
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步 创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};

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
            getCurrentSampleRateForOpensles(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    // 输出源
    SLDataSource slDataSource = {&android_queue, &pcm};

    LOGI("创建播放器");
    //第四步，创建播放器
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    // 创建音频播放对象AudioPlayer
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 1, ids, req);
    //初始化AudioPlayer
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    // 得到接口后调用，获取播放器接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

    // 注册回调缓冲区，获取缓冲队列接口，即音频输出的BufferQueue接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);

    //输出缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);

    LOGI("播放器 开始播放!");
    // 获取播放状态接口,设置为播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);

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