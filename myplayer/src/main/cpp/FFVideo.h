//
// Created by workw on 2020/12/5.
//

#ifndef FFMUSIC_FFVIDEO_H
#define FFMUSIC_FFVIDEO_H

#include "AVPacketQueue.h"
#include "FFCallJava.h"
#include "FFAudio.h"

#define CODEC_YUV 0
#define CODEC_MEDIACODEC 1

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

class FFVideo {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;//描述编解码器上下文
    AVCodecParameters *codecpar = NULL;// 包含音视频参数的结构体。很重要，可以用来获取音视频参数中的宽度、高度、采样率、编码格式等信息

    AVPacketQueue *queue = NULL;
    PlayStatus *playstatus = NULL;
    FFCallJava *callJava = NULL;

    pthread_mutex_t codecMutex;

    AVRational time_base;//时基。通过该值可以把PTS，DTS转化为真正的时间 PTS*time_base=真正的时间

    pthread_t thread_playvideo;

    FFAudio *audio = NULL;
    double clock = 0;
    double delayTime = 0;
    double defaultDelayTime = 0.04;

    int codectype = CODEC_YUV;

    AVBSFContext *abs_ctx = NULL;

public:
    FFVideo(PlayStatus *playStatus, FFCallJava *callJava);
    ~FFVideo();

    void play();

    void release();

    double getFrameDiffTime(AVFrame *avFrame);

    double getDelayTime(double diff);
};



#endif //FFMUSIC_FFVIDEO_H
