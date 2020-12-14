//
// Created by workw on 2019/5/27.
//

#include "AVPacketQueue.h"


AVPacketQueue::AVPacketQueue(PlayStatus *playStatus) {

    this->playStatus = playStatus;
    pthread_mutex_init(&mutexPacket, NULL);
    pthread_cond_init(&condPacket, NULL);
}

AVPacketQueue::~AVPacketQueue() {
    clearAvpacket();
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);
}

int AVPacketQueue::putAVpacket(AVPacket *packet) {

    pthread_mutex_lock(&mutexPacket);

    queuePacket.push(packet);
//    LOGI("Add a AVPacket to queue, count:%d", queuePacket.size());

    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);

    return 0;
}

/**
 * 从队列里面取出一个音频帧
 * @param packet
 * @return
 */
int AVPacketQueue::popAVpacket(AVPacket *packet) {

    pthread_mutex_lock(&mutexPacket);

    while (playStatus != NULL && !playStatus->exit){

        if (queuePacket.size() > 0 ){
            AVPacket *avPacket = queuePacket.front();
            if (av_packet_ref(packet, avPacket) == 0){
                queuePacket.pop();
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            //LOGI("从队列里面取出一个AVpacket，还剩下 %d 个", queuePacket.size());
            break;
        } else {
            pthread_cond_wait(&condPacket, &mutexPacket);
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}


int AVPacketQueue::getQueueSize() {

    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);

    return size;
}

void AVPacketQueue::clearAvpacket() {

    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);

    while (!queuePacket.empty()){
        AVPacket *packet = queuePacket.front();
        queuePacket.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&mutexPacket);
}

void AVPacketQueue::noticeQueue() {
    pthread_cond_signal(&condPacket);
}
