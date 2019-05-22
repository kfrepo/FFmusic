package com.wguet.myplayer;


public class Demo {

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

    public native void testFFmpeg();

    public native void normalThread();

    public native void mutexThread();






    private OnErrotListener onErrotListener;

    public void setOnErrotListener(OnErrotListener onErrotListener) {
        this.onErrotListener = onErrotListener;
    }

    public void onError(int code, String msg) {

        if (onErrotListener != null){
            onErrotListener.onError(code, msg);
        }
    }

    public interface OnErrotListener{
        void onError(int code, String msg);
    }

    public native  void callbackFromC();

}
