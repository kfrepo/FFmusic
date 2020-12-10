//
// Created by userwang on 2019/5/22.
//

#include "FFCallJava.h"


FFCallJava::FFCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *job) {

    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = *job;
    this->jobj = jniEnv->NewGlobalRef(jobj);// 全局引用,这种对象如不主动释放,它永远都不会被垃圾回收

    jclass  jlz = jniEnv->GetObjectClass(jobj);//通过对象获取这个类
    if(!jlz){
        if(LOG_DEBUG){
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
    jmid_load = env->GetMethodID(jlz, "onCallLoad", "(Z)V");
    jmid_timeinfo = env->GetMethodID(jlz, "onCallTimeInfo", "(II)V");
    jmid_error = env->GetMethodID(jlz, "onCallError", "(ILjava/lang/String;)V");
    jmid_complete = env->GetMethodID(jlz, "onCallComplete", "()V");
    jmid_valumedb = env->GetMethodID(jlz, "onCallValumeDB", "(I)V");
    jmie_pcmtoaac = env->GetMethodID(jlz, "encodecPcmToAAc", "(I[B)V");
    jmid_renderyuv = env->GetMethodID(jlz, "onCallRenderYUV", "(II[B[B[B)V");
    jmid_supportvideo = env->GetMethodID(jlz, "onCallIsSupportMediaCodec", "(Ljava/lang/String;)Z");
}

FFCallJava::~FFCallJava() {
}

void FFCallJava::onCallPrepared(int type) {

    if(type == MAIN_THREAD) {

        jniEnv->CallVoidMethod(jobj, jmid_prepared);
    } else if(type == CHILD_THREAD) {

        JNIEnv *jniEnv;

        //绑定当前线程到JavaVM，并获取的JNIEnv
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("get child thread jnienv worng");
            return;
        }

        jniEnv->CallVoidMethod(jobj, jmid_prepared);
        javaVM->DetachCurrentThread();
    }
}

void FFCallJava::onCallLoad(int type, bool load) {

    if(type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_load);
    }else if(type == CHILD_THREAD) {

        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("get child thread jnienv worng");
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
        javaVM->DetachCurrentThread();
    }
}

void FFCallJava::onCallTimeInfo(int type, int curr, int total) {
    if(type == MAIN_THREAD) {

        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
    }else if(type == CHILD_THREAD) {

        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){
            LOGE("call onCallTimeInfo worng");
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
        javaVM->DetachCurrentThread();
    }
}

void FFCallJava::onCallError(int type, int code, char *msg) {

    if(type == MAIN_THREAD){

        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else if(type == CHILD_THREAD){

        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){
            LOGE("call onCallError worng");
            return;
        }

        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }
}

void FFCallJava::onCallComplete(int type) {

    if(type == MAIN_THREAD){

        jniEnv->CallVoidMethod(jobj, jmid_complete);
    } else if(type == CHILD_THREAD){

        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){
            LOGE("call onCallComplete worng");
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_complete);
        javaVM->DetachCurrentThread();
    }
}

void FFCallJava::onCallValumeDB(int type, int db) {
    if(type == MAIN_THREAD){
        jniEnv->CallVoidMethod(jobj, jmid_valumedb, db);
    }else if(type == CHILD_THREAD){
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){
            LOGE("call onCallComplete worng");
            return;
        }
//        LOGI("db %d", db);
        jniEnv->CallVoidMethod(jobj, jmid_valumedb, db);
        javaVM->DetachCurrentThread();
    }
}

void FFCallJava::onCallPcmToAAc(int type, int size, void *buffer) {
    if (type == MAIN_THREAD){
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size,
                static_cast<const jbyte *>(buffer));
        jniEnv->CallVoidMethod(jobj, jmie_pcmtoaac, size, jbuffer);
        jniEnv->DeleteGlobalRef(jbuffer);
    } else if (type == CHILD_THREAD){
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){
            LOGE("call onCallPcmToAAc worng");
        }
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size,
                                   static_cast<const jbyte *>(buffer));

        jniEnv->CallVoidMethod(jobj, jmie_pcmtoaac, size, jbuffer);

        jniEnv->DeleteLocalRef(jbuffer);

        javaVM->DetachCurrentThread();
    }
}

void FFCallJava::onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){
        LOGE("call onCallRenderYUV worng");
        return;
    }
    jbyteArray y = jniEnv->NewByteArray(width * height);
    jniEnv->SetByteArrayRegion(y, 0, width*height, reinterpret_cast<const jbyte *>(fy));

    jbyteArray u = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(u, 0, width * height / 4, reinterpret_cast<const jbyte *>(fu));

    jbyteArray v = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(v, 0, width * height / 4, reinterpret_cast<const jbyte *>(fv));

    jniEnv->CallVoidMethod(jobj, jmid_renderyuv, width, height, y, u, v);

    jniEnv->DeleteLocalRef(y);
    jniEnv->DeleteLocalRef(u);
    jniEnv->DeleteLocalRef(v);

    javaVM->DetachCurrentThread();
}

bool FFCallJava::onCallIsSupportMediaCodec(const char *codecName) {
    bool support = false;
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK){
        LOGE("call onCallIsSupportMediaCodec worng");
        return support;
    }

    jstring type = jniEnv->NewStringUTF(codecName);
    support = jniEnv->CallBooleanMethod(jobj, jmid_supportvideo, type);
    jniEnv->DeleteLocalRef(type);
    javaVM->DetachCurrentThread();
    return support;
}
