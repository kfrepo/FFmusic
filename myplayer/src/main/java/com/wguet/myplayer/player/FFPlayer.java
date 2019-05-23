package com.wguet.myplayer.player;

import android.text.TextUtils;

import com.wguet.myplayer.listener.FFOnPreparedListener;

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

    private String source;//数据源

    private FFOnPreparedListener preparedListener;


    public void setSource(String source){
        this.source= source;
    }

    public void setPreparedListener(FFOnPreparedListener listener){
        this.preparedListener = listener;
    }

    public void prepared(){
        if (TextUtils.isEmpty(source)){
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {

                n_prepared(source);
            }
        }).start();
    }

    public void onCallPrepared(){

        if (preparedListener != null){
            preparedListener.onPrepared();
        }
    }

    public native void n_prepared(String source);

    public native void start();

}
