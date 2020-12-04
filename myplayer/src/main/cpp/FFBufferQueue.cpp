//
// Created by workw on 2020/12/3.
//

#include "FFBufferQueue.h"

FFBufferQueue::FFBufferQueue(PlayStatus *status) {
    playStatus = status;
    pthread_mutex_init(&mutexBuffer, NULL);
    pthread_cond_init(&condBuffer, NULL);
}

FFBufferQueue::~FFBufferQueue() {
    playStatus = NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
    LOGI("FFBufferQueue 释放完了");
}

int FFBufferQueue::putBuffer(SAMPLETYPE *buffer, int size) {
    pthread_mutex_lock(&mutexBuffer);
    PcmBean *pcmBean = new PcmBean(buffer, size);
    queueBuffer.push_back(pcmBean);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int FFBufferQueue::getBuffer(PcmBean **pPcmBean) {
    pthread_mutex_lock(&mutexBuffer);

    while(playStatus != NULL && !playStatus->exit){
        if(queueBuffer.size() > 0){
            *pPcmBean = queueBuffer.front();
            queueBuffer.pop_front();
            break;
        } else{
            if(!playStatus->exit){
                pthread_cond_wait(&condBuffer, &mutexBuffer);
            }
        }
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int FFBufferQueue::clearBuffer() {
    pthread_cond_signal(&condBuffer);
    pthread_mutex_lock(&mutexBuffer);
    while (!queueBuffer.empty()){
        PcmBean *pcmBean = queueBuffer.front();
        queueBuffer.pop_front();
        delete(pcmBean);
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

void FFBufferQueue::release() {
    LOGI("FFBufferQueue::release");
    noticeThread();
    clearBuffer();
    LOGI("FFBufferQueue::release success");
}

int FFBufferQueue::getBufferSize() {
    int size = 0;
    pthread_mutex_lock(&mutexBuffer);
    size = queueBuffer.size();
    pthread_mutex_unlock(&mutexBuffer);
    return size;
}

int FFBufferQueue::noticeThread() {
    pthread_cond_signal(&condBuffer);
    return 0;
}
