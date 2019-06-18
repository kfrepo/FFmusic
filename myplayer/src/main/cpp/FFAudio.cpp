//
// Created by workw on 2019/5/23.
//

#include <libswresample/swresample.h>
#include "FFAudio.h"

FFAudio::FFAudio(PlayStatus *playStatus) {
    this->playstatus = playStatus;
    queue = new AVPacketQueue(playstatus);

<<<<<<< HEAD
//    buffer = (uint8_t *) av_malloc(44100 *2 *2);
=======
    buffer = (uint8_t *) av_malloc(44100 *2 *2);
>>>>>>> b18df2e001e7b875347e71d03aa9c7b1d5a620c8

};

FFAudio::~FFAudio() {

};

void *decodPlay(void *data){

//    FFAudio *ffAudio = (FFAudio *) data;
//    ffAudio->resampleAudio();
//    pthread_exit(&ffAudio->thread_play);
}

void FFAudio::play(){
    pthread_create(&thread_play, NULL, decodPlay, this);
}

FILE *outFile = fopen("/mnt/sdcard/mymusic.pcm", "w");

int FFAudio::resampleAudio() {

    return  0;
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
            if (avFrame->channels > 0 && avFrame->channel_layout == 0)
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

            //针对每一帧音频的处理。把一帧帧的音频作相应的重采样
            /**
             * 参数1：音频重采样的上下文 参数2：输出的指针。传递的输出的数组 参数3：输出的样本数量，不是字节数。单通道的样本数量。 参数4：输入的数组，AVFrame解码出来的DATA 参数5：输入的单通道的样本数量。
             */
            int nb = swr_convert(
                    swr_ctx,
                    &buffer,
                    avFrame->nb_samples,
                    (const uint8_t **) avFrame->data,
                    avFrame->nb_samples
                    );

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            fwrite(buffer, 1, data_size, outFile);

            LOGE("data_size is %d", data_size);
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
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
    fclose(outFile);
    return data_size;
}