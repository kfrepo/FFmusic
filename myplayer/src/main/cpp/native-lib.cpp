#include <jni.h>
#include <string>
#include <pthread.h>
#include "AndroidLog.h"
#include "Listener.h"
#include "FFCallJava.h"
#include "MFFmpeg.h"
#include "Playstatus.h"
extern "C"{
#include "include/libavformat/avformat.h"
#include "include/libavcodec/avcodec.h"
#include "include/libavutil/avutil.h"
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_Demo_testFFmpeg(JNIEnv *env, jobject instance) {

    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL)
    {
        switch (c_temp->type)
        {
            case AVMEDIA_TYPE_VIDEO:
                LOGI("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGI("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGI("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
}


pthread_t thread;
void *threadCallBack(void *data){

    LOGI("thread threadCallBack !");
    pthread_exit(&thread);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_Demo_normalThread(JNIEnv *env, jobject instance) {

    pthread_create(&thread, NULL, threadCallBack, NULL);
}


/**
 * 生产者 消费者模型
 */
#include <queue>
#include <unistd.h>
pthread_t producer;//生产者线程
pthread_t customer;//消费者线程
pthread_mutex_t mutex;//线程锁
pthread_cond_t cond;//条件变量 是利用线程间共享的全局变量进行同步的一种机制

std::queue<int> queue;

void *producCallBack(void *data){

    while(1){

        pthread_mutex_lock(&mutex);
        queue.push(1);
        LOGI("生产者:生产一个产品,剩余产品%d", queue.size());
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
}

void *customCallBack(void *data){

    while(1){
        pthread_mutex_lock(&mutex);
        if (queue.size() > 0){
            queue.pop();
            LOGI("消费者:消费一个产品，剩余产品%d", queue.size());
        }else {
            LOGI("消费者:没有产品可以消费，等待中...");
            //用于线程阻塞等待，直到pthread_cond_signal发出条件信号后
            //才执行退出线程阻塞执行后面的操作
            pthread_cond_wait(&cond, &mutex);
        }

        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_Demo_mutexThread(JNIEnv *env, jobject instance) {

    for (int i = 0; i < 10; ++i) {
        queue.push(i);
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&producer, NULL, producCallBack, NULL);
    pthread_create(&customer, NULL, customCallBack, NULL);
}



/*********************************调用Java 方法***************************************/

_JavaVM *jvm = NULL;

Listener *javaListener;
pthread_t chidlThread;

void *childCallback(void *data) {

    Listener *javaListener1 = (Listener *) data;
    javaListener1->onError(0, 101, "c++ call java meid from child thread!");
    pthread_exit(&chidlThread);
}


//调用Java方法，非静态jobject instance ， 静态(JNIEnv *env, jclass type)
extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_Demo_callbackFromC(JNIEnv *env, jobject instance) {

    javaListener = new Listener(jvm, env, env->NewGlobalRef(instance));

    //主线程调用
//    javaListener->onError(1, 100, "c++ call java meid from main thread!");

    //子线程调用
    pthread_create(&chidlThread, NULL, childCallback, javaListener);
}


/**
 * 1.获取JVM对象 JavaVM是虚拟机在JNI中的表示，一个虚拟机中只有一个JavaVM对象，这个对象是线程共享的。JNIEnv类型是一个指向全部JNI方法的指针。该指针只在创建它的线程有效，不能跨线程传递。多线程无法共享。
 * 这个方法是在加载相应的.so包的时候，系统主动调用的
 * @param vm
 * @param reserved
 * @return
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void* reserved) {
    JNIEnv *env;
    jvm = vm;
    if(vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}

/**
 * 2.获取JVM对象
 */
//extern "C"
//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved){
//    jint result = -1;
//    javaVM = vm;
//    JNIEnv *env;
//    if(vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK)
//    {
//
//        return result;
//    }
//    return JNI_VERSION_1_4;
//
//}



/***********************音频播放*********************************/
FFCallJava *callJava = NULL;
MFFmpeg *mFFmpeg = NULL;

PlayStatus *playStatus = NULL;
bool nexit = true;
pthread_t thread_start;

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniPrepared(JNIEnv *env, jobject instance, jstring source_) {

    const char *source = env->GetStringUTFChars(source_, 0);

    if(mFFmpeg == NULL){
        if(callJava == NULL){
            callJava = new FFCallJava(jvm, env, &instance);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playStatus = new PlayStatus();
        mFFmpeg = new MFFmpeg(playStatus, callJava, source);
        mFFmpeg->parpared();
    }
//    env->ReleaseStringUTFChars(source_, source);
}

void *startCallBack(void *data){
    MFFmpeg *ffmpeg = (MFFmpeg *) data;
    ffmpeg->start();
    pthread_exit(&thread_start);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniStart(JNIEnv *env, jobject instance) {

    if(mFFmpeg != NULL){
        pthread_create(&thread_start, NULL, startCallBack, mFFmpeg);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniPause(JNIEnv *env, jobject instance) {

    if(mFFmpeg != NULL){
        mFFmpeg->pause();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniResume(JNIEnv *env, jobject instance) {

    if(mFFmpeg != NULL){
        mFFmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniStop(JNIEnv *env, jobject instance) {

    if (!nexit){
        return;
    }

    jclass jlz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(jlz, "onCallNext", "()V");

    nexit = false;
    if (mFFmpeg) {

        mFFmpeg->release();
        delete(mFFmpeg);
        mFFmpeg = NULL;

        if (callJava != NULL) {
            delete(callJava);
            callJava = NULL;
        }

        if (playStatus != NULL) {
            delete(playStatus);
            playStatus = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(instance, jmid_next);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniSeek(JNIEnv *env, jobject instance, jint seconds){

    if (mFFmpeg != NULL){
        mFFmpeg->seek(seconds);
    }
}