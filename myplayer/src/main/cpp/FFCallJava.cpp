//
// Created by userwang on 2019/5/22.
//

#include "FFCallJava.h"


FFCallJava::FFCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *job) {

    this->javaVM = javaVM;
    this->jniEnV = env;
    this->jobj = *job;
    this->jobj = jniEnV->NewGlobalRef(jobj);// 全局引用,这种对象如不主动释放,它永远都不会被垃圾回收

    jclass  jlz = jniEnV->GetObjectClass(jobj);//通过对象获取这个类
    if(!jlz){
        if(LOG_DEBUG){
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
    jmid_load = env->GetMethodID(jlz, "onCallLoad", "(Z)V");
}


void FFCallJava::onCallPrepared(int type) {

    if(type == MAIN_THREAD){
        jniEnV->CallVoidMethod(jobj, jmid_prepared);
    }
    else if(type == CHILD_THREAD){

        JNIEnv *jniEnv;

        //绑定当前线程到JavaVM，并获取的JNIEnv
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng");
            }
            return;
        }

        jniEnv->CallVoidMethod(jobj, jmid_prepared);
        javaVM->DetachCurrentThread();
    }
}

void FFCallJava::onCallLoad(int type, bool load) {
    if(type == MAIN_THREAD){
        jniEnV->CallVoidMethod(jobj, jmid_load);
    }else if(type == CHILD_THREAD){

        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){

             LOGE("get child thread jnienv worng");
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
        javaVM->DetachCurrentThread();
    }
}
