//
// Created by userwang on 2019/5/22.
//

#ifndef FFMUSIC_FFCALLJAVA_H
#define FFMUSIC_FFCALLJAVA_H

#include "jni.h"
#include <linux/stddef.h>

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

public:
    FFCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *job);
    ~FFCallJava();

    void onCallPrepared(int type);

    void onCallLoad(int type, bool load);

    void onCallTimeInfo(int type, int curr, int total);

    void onCallError(int type, int code, char *msg);

    void onCallComplete(int type);
};


#endif //FFMUSIC_FFCALLJAVA_H
