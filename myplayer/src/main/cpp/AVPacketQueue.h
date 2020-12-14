//
// Created by workw on 2019/5/27.
//

#ifndef FFMUSIC_AVPACKETQUEUE_H
#define FFMUSIC_AVPACKETQUEUE_H


#include <queue>
#include "pthread.h"
#include "PlayStatus.h"
#include "AndroidLog.h"

extern "C"{
#include <libavcodec/avcodec.h>
};


class AVPacketQueue {

public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;//线程锁
    pthread_cond_t condPacket;//条件对象
    PlayStatus *playStatus = NULL;

public:
    AVPacketQueue(PlayStatus *playStatus);
    ~AVPacketQueue();

    int putAVpacket(AVPacket *packet);
    int popAVpacket(AVPacket *packet);

    int getQueueSize();

    void clearAvpacket();

    void noticeQueue();
};


#endif //FFMUSIC_AVPACKETQUEUE_H
