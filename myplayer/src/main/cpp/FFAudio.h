//
// Created by workw on 2019/5/23.
//

#ifndef FFMUSIC_FFAUDIO_H
#define FFMUSIC_FFAUDIO_H

#include "AVPacketQueue.h"
#include "PlayStatus.h"
#include "FFCallJava.h"
#include "SoundTouch.h"

using namespace soundtouch;

extern "C"{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};


class FFAudio {

public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;//描述编解码器上下文
    AVCodecParameters *codecpar = NULL;// 包含音视频参数的结构体。很重要，可以用来获取音视频参数中的宽度、高度、采样率、编码格式等信息

    AVPacketQueue *queue = NULL;
    PlayStatus *playstatus = NULL;
    FFCallJava *callJava = NULL;

    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = -1;
    uint8_t  *buffer = NULL;
    int data_size = 0;

    int sample_rate = 0;

    int duration = 0;
    AVRational time_base;
    double clock = 0;//总的播放时长
    double now_time = 0;//当前frame时间
    double last_tiem = 0; //上一次调用时间

    //引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //PCM
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;
    SLVolumeItf pcmVolumePlay = NULL;
    SLMuteSoloItf pcmMutePlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    SoundTouch *soundTouch =NULL;
    SAMPLETYPE *sampleBuffer = NULL;

public:
    FFAudio(PlayStatus *playStatus, int sample_rate, FFCallJava *callJava);
    ~FFAudio();

    int resampleAudio();

    void initOpenSLES();

    int getCurrentSampleRateForOpensles(int sample_rate);

    void play();
    void pause();
    void resume();
    void stop();
    void release();

    void setVolume(int percent);

    void setMute(int mute);


};


#endif //FFMUSIC_FFAUDIO_H
/***
 * AVCodecParameters
enum AVMediaType codec_type; 　　　// 编码类型。说明这段流数据究竟是音频还是视频。
enum AVCodecID codec_id     　　　// 编码格式。说明这段流的编码格式，h264，MPEG4, MJPEG，etc...

uint32_t  codecTag;        //  一般不用
int format;                //  格式。对于视频来说指的就是像素格式(YUV420,YUV422...)，对于音频来说，指的就是音频的采样格式。
int width, int height;     // 视频的宽高，只有视频有
uint64_t channel_layout;   // 取默认值即可
int channels;               // 声道数
int sample_rate;            // 样本率
int frame_size;             // 只针对音频，一帧音频的大小
 ***/