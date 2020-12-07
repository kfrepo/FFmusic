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
    LOGI("FFVideo::playVideo");
    FFVideo *video = static_cast<FFVideo *>(data);
    while (video->playstatus != NULL && !video->playstatus->exit){
        if (video->playstatus->seek){
            av_usleep(1000 * 100);
            continue;
        }

        if (video->queue->getQueueSize() == 0){
            if (!video->playstatus->load){
                video->playstatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else{
            if(video->playstatus->load){
                video->playstatus->load = false;
                video->callJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        AVPacket  *avPacket = av_packet_alloc();
        //解码渲染
        if (video->queue->popAVpacket(avPacket) != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        if (avcodec_send_packet(video->avCodecContext, avPacket) != 0){
            LOGE("avcodec_send_packet error!");
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(video->avCodecContext, avFrame) != 0){
            LOGE("avcodec_receive_frame error!");
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        //LOGI("解码 video AVframe %dx%d format %d %d %d", avFrame->width, avFrame->height, avFrame->format, avFrame->pict_type, avFrame->pkt_size);

        if (avFrame->format == AV_PIX_FMT_YUV420P){
            video->callJava->onCallRenderYUV(
                    avFrame->width,
                    avFrame->height,
                    avFrame->data[0],
                    avFrame->data[1],
                    avFrame->data[2]);
        } else{
            AVFrame *frameYUV420P = av_frame_alloc();
            //通过指定像素格式、图像宽、图像高来计算所需的内存大小
            int num = av_image_get_buffer_size(
                    AV_PIX_FMT_YUV420P,
                    avFrame->width,
                    avFrame->height,
                    1);
            uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));

            av_image_fill_arrays(
                    frameYUV420P->data,
                    frameYUV420P->linesize,
                    buffer,
                    AV_PIX_FMT_YUV420P,
                    avFrame->width,
                    avFrame->height,
                    1);

            SwsContext *swsContext = sws_getContext(
                    avFrame->width,
                    avFrame->height,
                    video->avCodecContext->pix_fmt,
                    avFrame->width,
                    avFrame->height,
                    AV_PIX_FMT_YUV420P,
                    SWS_BICUBIC, NULL, NULL, NULL);

            if (!swsContext){
                av_frame_free(&frameYUV420P);
                av_free(frameYUV420P);
                av_free(buffer);
                continue;
            }
            sws_scale(
                    swsContext,
                    reinterpret_cast<const uint8_t *const *>(avFrame->data),
                    avFrame->linesize,
                    0,
                    avFrame->height,
                    frameYUV420P->data,
                    frameYUV420P->linesize
                    );

            video->callJava->onCallRenderYUV(
                    avFrame->width,
                    avFrame->height,
                    avFrame->data[0],
                    avFrame->data[1],
                    avFrame->data[2]);

            av_frame_free(&frameYUV420P);
            av_free(frameYUV420P);
            av_free(buffer);
            sws_freeContext(swsContext);
        }

        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;

        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    pthread_exit(&video->thread_playvideo);
}

void FFVideo::play() {
    pthread_create(&thread_playvideo, NULL, playVideo, this);
}

void FFVideo::release() {
    if (queue != NULL){
        delete(queue);
        queue = NULL;
    }

    if (avCodecContext != NULL){
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }

    if (playstatus != NULL){
        playstatus = NULL;
    }
    if (callJava != NULL){
        callJava = NULL;
    }
}
