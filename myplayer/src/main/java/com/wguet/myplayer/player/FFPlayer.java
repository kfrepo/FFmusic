package com.wguet.myplayer.player;

import android.text.TextUtils;

import com.wguet.myplayer.TimeInfoBean;
import com.wguet.myplayer.listener.FFOnCompleteListener;
import com.wguet.myplayer.listener.FFOnErrorListener;
import com.wguet.myplayer.listener.FFOnLoadListener;
import com.wguet.myplayer.listener.FFOnPauseResumeListener;
import com.wguet.myplayer.listener.FFOnPreparedListener;
import com.wguet.myplayer.listener.FFOnTimeInfoListener;
import com.wguet.myplayer.util.LogUtil;

/**
 * @author workw
 */
@SuppressWarnings({"ALL", "AlibabaAvoidManuallyCreateThread"})
public class FFPlayer {

    private static final String TAG = FFPlayer.class.getName();
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

    private TimeInfoBean timeInfoBean;

    private FFOnPreparedListener preparedListener;
    private FFOnLoadListener ffOnLoadListener;
    private FFOnPauseResumeListener ffOnPauseResumeListener;
    private FFOnTimeInfoListener ffOnTimeInfoListener;
    private FFOnErrorListener ffOnErrorListener;
    private FFOnCompleteListener ffOnCompleteListener;

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

    public void setFfOnTimeInfoListener(FFOnTimeInfoListener listener){
        this.ffOnTimeInfoListener = listener;
    }

    public void setFfOnErrorListener(FFOnErrorListener listener){
        this.ffOnErrorListener = listener;
    }

    public void setFfOnCompleteListener(FFOnCompleteListener listener){
        this.ffOnCompleteListener = listener;
    }


    public void prepared(){
        if (TextUtils.isEmpty(source)){
            LogUtil.e("source is null!");
            return;
        }
//        onCallLoad(true);

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

    public void stop() {
        timeInfoBean = null;
        new Thread(new Runnable() {
            @Override
            public void run() {
                jniStop();
            }
        }).start();
    }

    public void seek(int seconds){
        jniSeek(seconds);
    }


    /**
     * c++回调java的方法
     */
    public void onCallPrepared(){
        if (preparedListener != null){
            LogUtil.d(TAG, "JNI 回调 onCallPrepared");
            preparedListener.onPrepared();
        }
    }

    public void onCallLoad(boolean load) {
        if(ffOnLoadListener != null) {
            LogUtil.d(TAG, "JNI 回调 onCallLoad load:" + load);
            ffOnLoadListener.onLoad(load);
        }
    }

    public void onCallTimeInfo(int currentTime, int totalTime){
        if(ffOnTimeInfoListener != null){
            if (timeInfoBean == null){
                timeInfoBean = new TimeInfoBean();
            }
            LogUtil.d(TAG, "JNI 回调 onCallTimeInfo currentTime:" + currentTime);
            timeInfoBean.setCurrentTime(currentTime);
            timeInfoBean.setTotalTime(totalTime);
            ffOnTimeInfoListener.onTimeInfo(timeInfoBean);
        }
    }

    public void onCallError(int code, String msg) {
        if(ffOnErrorListener != null) {
            stop();
            ffOnErrorListener.onError(code, msg);
        }
    }

    public void onCallComplete(){
        if (ffOnCompleteListener != null){
            stop();
            ffOnCompleteListener.onComplete();
        }
    }

    private native void jniPrepared(String source);
    private native void jniStart();
    private native void jniPause();
    private native void jniResume();
    private native void jniStop();
    private native void jniSeek(int seconds);

}
