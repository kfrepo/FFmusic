//
// Created by userwang on 2019/5/22.
//

#include "FFCallJava.h"
#include "AndroidLog.h"

FFCallJava::FFCallJava(_JavaVM *javaVM, JNIEnv *jniEnv, jobject *job) {

    this->javaVM = javaVM;
    this->jniEnv = jniEnv;
    this->jobj = *job;
    this->jobj = jniEnv->NewGlobalRef(jobj);// 全局引用,这种对象如不主动释放,它永远都不会被垃圾回收

    jclass  jlz = jniEnv->GetObjectClass(jobj);//通过对象获取这个类
    if(!jlz){
        if(LOG_DEBUG){
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_prepared = jniEnv->GetMethodID(jlz, "onCallPrepared", "()V");
}

FFCallJava::~FFCallJava() {

}


//???
void FFCallJava::onCallPrepared(int type) {

    if(type == MAIN_THREAD){
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        //绑定当前线程到JavaVM，并获取的JNIEnv
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
        javaVM->DetachCurrentThread();
    }
}
