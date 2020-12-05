//
// Created by workw on 2020/12/5.
//

#include "FFVideo.h"

FFVideo::FFVideo(PlayStatus *playStatus, FFCallJava *callJava) {
    this->playstatus = playStatus;
    this->callJava = callJava;
    queue = new AVPacketQueue(playStatus);
}

FFVideo::~FFVideo() {

}

void *playVideo(void *data){
    FFVideo *video = static_cast<FFVideo *>(data);
    while (video->playstatus != NULL && !video->playstatus->exit){
        AVPacket  *avPacket = av_packet_alloc();
        //解码渲染
        if (video->queue->popAVpacket(avPacket) != 0){

        }

        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    pthread_exit(&video->thread_playvideo);
}

void FFVideo::play() {
    pthread_create(&thread_playvideo, NULL, playVideo, this);
}
