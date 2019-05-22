//
// Created by userwang on 2019/4/14.
//


#include "../../../../../../Android/NDK/android-ndk-r14b/platforms/android-17/arch-x86/usr/include/jni.h"
#include "Listener.h"

Listener::Listener(JavaVM *vm, _JNIEnv *env, jobject obj) {

    javaVM = vm;
    jniEnv = env;
    jobj = obj;

    jclass  clz = env->GetObjectClass(jobj);
    /**
     *  1、jclass FindClass(const char* clsName):通过类的全名来获取jclass,
            jclass str = env->FindClass("com/wguet/myplayer/Demo");获取Java中的String对象的class对象。
        2、jclass GetObjectClass(jobject obj):通过对象实例来获取jclass,相当于java中的getClass方法
        3、jclass GetSuperClass(jclass obj):通过jclass可以获取其父类的jclass对象

     */

    if(!clz){
        return ;
    }

    jmid = env->GetMethodID(clz, "onError", "(ILjava/lang/String;)V");
    if(!jmid)
        return;
}

void Listener::onError(int type, int code, const char *msg) {

    /*0 子线程  1 主线程*/
    if(type == 0){
        JNIEnv *env;
        javaVM->AttachCurrentThread(&env, 0);
        jstring jmsg = env->NewStringUTF(msg);
        env->CallVoidMethod(jobj, jmid, code, jmsg);
        env->DeleteLocalRef(jmsg);

        javaVM->DetachCurrentThread();
    }else if(type == 1){
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    }

}
