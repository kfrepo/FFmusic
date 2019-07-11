package com.wguet.myplayer.player;

import android.text.TextUtils;

import com.wguet.myplayer.listener.FFOnLoadListener;
import com.wguet.myplayer.listener.FFOnPauseResumeListener;
import com.wguet.myplayer.listener.FFOnPreparedListener;
import com.wguet.myplayer.util.LogUtil;

/**
 * @author workw
 */
@SuppressWarnings({"ALL", "AlibabaAvoidManuallyCreateThread"})
public class FFPlayer {

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }

    private String source;

    private FFOnPreparedListener preparedListener;
    private FFOnLoadListener ffOnLoadListener;
    private FFOnPauseResumeListener ffOnPauseResumeListener;

    public void setSource(String source){
        this.source= source;
    }

    public void setPreparedListener(FFOnPreparedListener listener){
        this.preparedListener = listener;
    }
    public void setFfOnLoadListener(FFOnLoadListener listener){
        this.ffOnLoadListener = listener;
    }
    public void setFfOnPauseResumeListener(FFOnPauseResumeListener listener){
        this.ffOnPauseResumeListener = listener;
    }


    public void prepared(){
        if (TextUtils.isEmpty(source)){
            LogUtil.e("source is null!");
            return;
        }
        onCallLoad(true);

        new Thread(new Runnable() {
            @Override
            public void run() {

                jniPrepared(source);
            }
        }).start();
    }

    public void start(){
        if(TextUtils.isEmpty(source)){
            LogUtil.d("source is empty!");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                jniStart();
            }
        }).start();
    }

    public void pause() {
        jniPause();
        if(ffOnPauseResumeListener != null) {
            ffOnPauseResumeListener.onPause(true);
        }
    }

    public void resume() {
        jniResume();
        if(ffOnPauseResumeListener != null) {
            ffOnPauseResumeListener.onPause(false);
        }
    }




    /**
     * c++回调java的方法
     */
    public void onCallPrepared(){

        if (preparedListener != null){
            preparedListener.onPrepared();
        }
    }

    public void onCallLoad(boolean load) {
        if(ffOnLoadListener != null) {
            ffOnLoadListener.onLoad(load);
        }
    }

    public native void jniPrepared(String source);

    public native void jniStart();
    private native void jniPause();
    private native void jniResume();
}
