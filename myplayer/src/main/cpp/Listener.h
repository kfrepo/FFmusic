
#include "../../../../../../Android/NDK/android-ndk-r14b/platforms/android-17/arch-arm/usr/include/jni.h"


#ifndef FFMUSIC_LISTENER_H  //先测试x是否被宏定义过
#define FFMUSIC_LISTENER_H


class Listener {

public:
    JavaVM *javaVM;
    _JNIEnv *jniEnv;
    jobject  jobj;
    jmethodID jmid;


public:

    Listener(JavaVM *vm, _JNIEnv *env, jobject obj);//构造函数
    ~Listener();//析构函数是类的一种特殊的成员函数，它会在每次删除所创建的对象时执行

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
