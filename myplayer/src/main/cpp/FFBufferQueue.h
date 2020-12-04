//
// Created by workw on 2020/12/3.
//

#ifndef FFMUSIC_FFBUFFERQUEUE_H
#define FFMUSIC_FFBUFFERQUEUE_H

#include <deque>
#include "PcmBean.h"
#include "PlayStatus.h"
#include "AndroidLog.h"

extern "C"{
#include <libavcodec/avcodec.h>
#include <pthread.h>
};

class FFBufferQueue {
public:
    std::deque<PcmBean *> queueBuffer;
    pthread_mutex_t  mutexBuffer;
    pthread_cond_t condBuffer;
    PlayStatus *playStatus = NULL;

public:
    FFBufferQueue(PlayStatus *status);
    ~FFBufferQueue();

    int putBuffer(SAMPLETYPE *buffer, int size);
    int getBuffer(PcmBean **pPcmBean);
    int clearBuffer();

    void release();
    int getBufferSize();

    int noticeThread();
};


#endif //FFMUSIC_FFBUFFERQUEUE_H
