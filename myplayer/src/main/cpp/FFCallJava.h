//
// Created by userwang on 2019/5/22.
//

#ifndef FFMUSIC_FFCALLJAVA_H
#define FFMUSIC_FFCALLJAVA_H

#include "jni.h"
#include <linux/stddef.h>
#include <stdint.h>

#include "AndroidLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1

class FFCallJava {
public:
    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID jmid_prepared;
    jmethodID jmid_load;
    jmethodID jmid_timeinfo;
    jmethodID jmid_error;
    jmethodID jmid_complete;
    jmethodID jmid_valumedb;
    jmethodID jmie_pcmtoaac;
    jmethodID jmid_renderyuv;
    jmethodID jmid_supportvideo;
    jmethodID jmid_initmediacodec;
    jmethodID jmid_decodeavpacket;

public:
    FFCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *job);
    ~FFCallJava();

    void onCallPrepared(int type);

    void onCallLoad(int type, bool load);

    void onCallTimeInfo(int type, int curr, int total);

    void onCallError(int type, int code, char *msg);

    void onCallComplete(int type);

    void onCallValumeDB(int type, int db);

    void onCallPcmToAAc(int type, int size, void *buffer);

    void onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv);

    bool onCallIsSupportMediaCodec(const char *codecName);

    void onCallInitMediacodec(const char *mime, int width, int height, int csd0_size, int csd1_size, uint8_t *csd_0, uint8_t *csd_1);

    void onCallDecodeAVPacket(int datasize, uint8_t *data);
};


#endif //FFMUSIC_FFCALLJAVA_H
