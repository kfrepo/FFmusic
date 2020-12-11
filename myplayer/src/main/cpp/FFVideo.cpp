//
// Created by workw on 2020/12/5.
//

#include "FFVideo.h"

FFVideo::FFVideo(PlayStatus *playStatus, FFCallJava *callJava) {
    this->playstatus = playStatus;
    this->callJava = callJava;
    queue = new AVPacketQueue(playStatus);
    pthread_mutex_init(&codecMutex, NULL);
}

FFVideo::~FFVideo() {
    pthread_mutex_destroy(&codecMutex);
}

void *playVideo(void *data){

    LOGI("FFVideo::playVideo");
    FFVideo *video = static_cast<FFVideo *>(data);
    while (video->playstatus != NULL && !video->playstatus->exit){

        if (video->playstatus->seek){
            av_usleep(1000 * 100);
            continue;
        }
        if (video->playstatus->pause){
            av_usleep(1000 * 100);
            continue;
        }

//        LOGI("FFVideo video->queue->getQueueSize() %d", video->queue->getQueueSize());
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

        AVPacket *avPacket = av_packet_alloc();
        //解码渲染
        if (video->queue->popAVpacket(avPacket) != 0){
            LOGE("FFVideo video->queue->popAVpacket(avPacket) error");
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        if (video->codectype == CODEC_MEDIACODEC){
            // 硬解码
            if (av_bsf_send_packet(video->abs_ctx, avPacket) != 0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            while (av_bsf_receive_packet(video->abs_ctx, avPacket) == 0){
                LOGI("video 硬解码 开始解码");
                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }
            avPacket = NULL;

        } else if (video->codectype == CODEC_YUV){

            pthread_mutex_lock(&video->codecMutex);
            //发送数据到ffmepg，放到解码队列中
            int res = avcodec_send_packet(video->avCodecContext, avPacket);
            if (res != 0){
                LOGE("avcodec_send_packet error %d!", res);
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }

            AVFrame *avFrame = av_frame_alloc();
            // 从解码器中获取1个解码的输出数据
            res = avcodec_receive_frame(video->avCodecContext, avFrame);
            if (res != 0){
                //AVERROR(EAGAIN);
                LOGE("avcodec_receive_frame error %d!", res);
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            //LOGI("解码 video AVframe %dx%d format %d %d %d", avFrame->width, avFrame->height, avFrame->format, avFrame->pict_type, avFrame->pkt_size);

            if (avFrame->format == AV_PIX_FMT_YUV420P){
                double diff = video->getFrameDiffTime(avFrame);
                double needDelayTime = video->getDelayTime(diff);
//            LOGI("diff is %f, needDelayTime is %f", diff, needDelayTime);
                av_usleep(needDelayTime * 1000 * 1000);

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
                    pthread_mutex_unlock(&video->codecMutex);
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
                double diff = video->getFrameDiffTime(avFrame);
                LOGI("convert frameYUV420P diff is %f", diff);

                av_usleep(video->getDelayTime(diff) * 1000 * 1000);

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
            pthread_mutex_unlock(&video->codecMutex);
        }


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
    if(abs_ctx != NULL){
        av_bsf_free(&abs_ctx);
        abs_ctx = NULL;
    }
    if (avCodecContext != NULL){
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
        pthread_mutex_unlock(&codecMutex);
    }

    if (playstatus != NULL){
        playstatus = NULL;
    }
    if (callJava != NULL){
        callJava = NULL;
    }
}

double FFVideo::getFrameDiffTime(AVFrame *avFrame) {

    double pts = av_frame_get_best_effort_timestamp(avFrame);
    //LOGI("getFrameDiffTime pts %f time_base %d %d", pts, time_base.num, time_base.den);
    if (pts == AV_NOPTS_VALUE){
        pts = 0;
    }

    pts *= av_q2d(time_base);
    if(pts > 0){
        clock = pts;
//        LOGI("video frame clock %f, audio clock %f", clock, audio->clock);
    }

    double diff = audio->clock - clock;
    return diff;
}

double FFVideo::getDelayTime(double diff) {
    if (diff > 0.003){
        //音频快
        delayTime = delayTime * 2 / 3;
        if(delayTime < defaultDelayTime / 2){
            delayTime = defaultDelayTime * 2 / 3;
        }else if(delayTime > defaultDelayTime * 2){
            delayTime = defaultDelayTime * 2;
        }
    } else if(diff < - 0.003){
        //视频快
        delayTime = delayTime * 3 / 2;
        if(delayTime < defaultDelayTime / 2){
            delayTime = defaultDelayTime * 2 / 3;
        }
        else if(delayTime > defaultDelayTime * 2){
            delayTime = defaultDelayTime * 2;
        }
    }else if(diff == 0.003){

    }

    if(diff >= 0.5){
        delayTime = 0;
    }else if(diff <= -0.5){
        delayTime = defaultDelayTime * 2;
    }

    if(fabs(diff) >= 10){
        delayTime = defaultDelayTime;
    }
    return delayTime;
}
