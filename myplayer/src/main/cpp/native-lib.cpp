#include <jni.h>
#include <string>
#include <pthread.h>
#include "AndroidLog.h"
#include "Listener.h"
#include "FFCallJava.h"
#include "MFFmpeg.h"

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

JavaVM *jvm;

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


/*获取JVM对象*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void* reserved) {
    JNIEnv *env;
    jvm = vm;
    if(vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}



/***********************音频播放*********************************/
FFCallJava *callJava = NULL;
_JavaVM *javaVM = NULL;
MFFmpeg *mfFmpeg = NULL;


extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_n_1prepared(JNIEnv *env, jobject instance,
                                                    jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if(mfFmpeg == NULL){
        if(callJava == NULL){
            callJava = new FFCallJava(javaVM, env, &instance);
        }
        mfFmpeg = new MFFmpeg(callJava, source);
        mfFmpeg->parpared();
    }

    env->ReleaseStringUTFChars(source_, source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_start(JNIEnv *env, jobject instance) {

    // TODO

}
