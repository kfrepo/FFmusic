#include "jni.h"

#ifndef FFMUSIC_LISTENER_H  //先测试x是否被宏定义过
#define FFMUSIC_LISTENER_H

class Listener {

public:
    JavaVM *javaVM;
    _JNIEnv *jniEnv;
    jobject  jobj;
    jmethodID jmid;

public:
    Listener(JavaVM *vm, _JNIEnv *env, jobject obj);
    ~Listener();

    /**
     * 1:主线程
     * 0：子线程
     * @param type
     * @param code
     * @param msg
     */
    void onError(int type, int code, const char *msg);
};

#endif //FFMUSIC_LISTENER_H
