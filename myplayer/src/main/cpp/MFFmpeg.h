//
// Created by workw on 2019/5/23.
//

#ifndef FFMUSIC_MFFMPEG_H
#define FFMUSIC_MFFMPEG_H

#include "FFCallJava.h"
#include "pthread.h"
#include "FFAudio.h"
#include "FFVideo.h"
#include "PlayStatus.h"

extern "C"
{
#include "libavformat/avformat.h"
#include <libavutil/time.h>
};

class MFFmpeg {

public:
    FFCallJava *callJava;
    const char* url;
    pthread_t decodeThread;
    AVFormatContext *pAVFormatCtx;//整个媒体流的处理流程中都会用到的对象,媒体文件或媒体流的构成和基本信息
    FFAudio *audio = NULL;
    FFVideo *video = NULL;
    PlayStatus *playstatus = NULL;

    pthread_mutex_t init_mutex;
    bool exit = false;

    int duration = 0;
    pthread_mutex_t seek_mutex;

public:
    MFFmpeg(PlayStatus *playStatus, FFCallJava *callJava, const char *url);
    ~MFFmpeg();

    void decodeFFmpegThread();

    int getCodecContext(AVCodecParameters *codecpar, AVCodecContext **avCodecContext);

    void parpared();
    void start();
    void pause();
    void resume();
    void release();
    void seek(int64_t seconds);

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getSampleRate();

    void startStopRecord(bool start);
};


#endif //FFMUSIC_MFFMPEG_H
