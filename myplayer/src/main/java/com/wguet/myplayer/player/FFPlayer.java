package com.wguet.myplayer.player;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.view.Surface;

import com.wguet.myplayer.TimeInfoBean;
import com.wguet.myplayer.listener.FFOnCompleteListener;
import com.wguet.myplayer.listener.FFOnErrorListener;
import com.wguet.myplayer.listener.FFOnLoadListener;
import com.wguet.myplayer.listener.FFOnPauseResumeListener;
import com.wguet.myplayer.listener.FFOnPreparedListener;
import com.wguet.myplayer.listener.FFOnTimeInfoListener;
import com.wguet.myplayer.listener.FFOnVolumeDBListener;
import com.wguet.myplayer.opengl.MGLSurfaceView;
import com.wguet.myplayer.opengl.MRender;
import com.wguet.myplayer.util.LogUtil;
import com.wguet.myplayer.util.VideoSupportUtil;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @author workw
 */
public class FFPlayer {

    private static final String TAG = "FFPlayer";
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
    private boolean playNext;
    private TimeInfoBean timeInfoBean;
    private static boolean initMediaCodec = false;

    private MGLSurfaceView mglSurfaceView;

    private FFOnPreparedListener preparedListener;
    private FFOnLoadListener ffOnLoadListener;
    private FFOnPauseResumeListener ffOnPauseResumeListener;
    private FFOnTimeInfoListener ffOnTimeInfoListener;
    private FFOnErrorListener ffOnErrorListener;
    private FFOnCompleteListener ffOnCompleteListener;
    private FFOnVolumeDBListener ffOnVolumeDBListener;

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
    public void setFfOnVolumeDBListener(FFOnVolumeDBListener listener){
        this.ffOnVolumeDBListener = listener;
    }

    public void prepared(){
        if (TextUtils.isEmpty(source)){
            LogUtil.e(TAG,"source is null!");
            return;
        }
//        onCallLoad(true);

        new Thread(new Runnable() {
            @Override
            public void run() {
                LogUtil.e(TAG,"prepared call jniPrepared " + source);
                jniPrepared(source);
            }
        }).start();
    }

    public void start(){
        if(TextUtils.isEmpty(source)){
            LogUtil.d(TAG, "source is empty!");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                setPitch(pitch);
                setSpeed(speed);
                setVolume(2);
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
        audioStopRecord();
        new Thread(new Runnable() {
            @Override
            public void run() {
                jniStop();
                releaseMediacodec();
            }
        }).start();
    }

    public void seek(int seconds){
        jniSeek(seconds);
    }

    public void playNext(String url) {
        source = url;
        playNext = true;
        stop();
    }

    public int getDuration() {
        int duration = jniDuration();
//        LogUtil.d(TAG, "音频时长 " + duration);
        return duration;
    }

    public void setVolume(int percent) {
        jniSetVolume(percent);
    }

    /**
     * 声道控制
     * @param mute
     */
    public void setMute(int mute) {
        jniSetMute(mute);
    }

    private static float speed = 1.0f;
    private static float pitch = 1.0f;
    /**
     * 变调
     * @param p
     */
    public void setPitch(float p) {
        pitch = p;
        jniSetPitch(pitch);
    }

    public void setSpeed(float s) {
        speed = s;
        jniSetSpeed(speed);
    }

    public void audioStartRecord(File outfile) {
        if (!initMediaCodec){
            if(jniSamplerate() > 0) {
                initMediaCodec = true;
                initMediacodec(jniSamplerate(), outfile);
                jniStartStopRecord(true);
                LogUtil.d(TAG,"开始录制");
            }
        }
    }

    public void audioStopRecord() {
        if(initMediaCodec) {
            jniStartStopRecord(false);
            releaseAudioMediacodec();
            LogUtil.d(TAG,"音频完成录制！");
        }
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
            //LogUtil.d(TAG, "JNI 回调 onCallLoad load:" + load);
            ffOnLoadListener.onLoad(load);
        }
    }

    public void onCallTimeInfo(int currentTime, int totalTime){
        if(ffOnTimeInfoListener != null){
            if (timeInfoBean == null){
                timeInfoBean = new TimeInfoBean();
            }
//            LogUtil.d(TAG, "JNI 回调 onCallTimeInfo currentTime:" + currentTime);
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

    public void onCallNext() {
        if(playNext) {
            playNext = false;
            prepared();
        }
    }

    public void onCallValumeDB(int db) {
        if (ffOnVolumeDBListener != null){
            ffOnVolumeDBListener.onDbValue(db);
        }
    }

    public void onCallRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
        //LogUtil.d(TAG, String.format("获取到视频的yuv数据 %dx%d y:%d u:%d v:%d", width, height, y.length, u.length, v.length));
        if (mglSurfaceView != null){
            mglSurfaceView.getMRender().setRenderType(MRender.RENDER_YUV);
            mglSurfaceView.setYUVData(width, height, y, u, v);
        }
    }

    public boolean onCallIsSupportMediaCodec(String codecName) {
        return VideoSupportUtil.isSupportCodec(codecName);
    }

    private native void jniPrepared(String source);
    private native void jniStart();
    private native void jniPause();
    private native void jniResume();
    private native void jniStop();
    private native void jniSeek(int seconds);

    private native int jniDuration();
    private native void jniSetVolume(int percent);
    private native void jniSetMute(int mute);

    private native void jniSetPitch(float pitch);
    private native void jniSetSpeed(float speed);
    private native int jniSamplerate();
    private native void jniStartStopRecord(boolean start);

    //mediacodec

    private MediaFormat encoderFormat = null;
    private MediaCodec encoder = null;
    private FileOutputStream outputStream = null;
    private MediaCodec.BufferInfo info = null;
    private int perpcmsize = 0;
    private byte[] outByteBuffer = null;
    private int aacsamplerate = 4;

    private void initMediacodec(int samperate, File outfile) {
        try {
            aacsamplerate = getADTSsamplerate(samperate);
            //格式 采样率 通道
            encoderFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, samperate, 2);
            //码率
            encoderFormat.setInteger(MediaFormat.KEY_BIT_RATE, 128*1000);
            //
            encoderFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            //输入数据缓冲区的最大大小
            encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);
            encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            info = new MediaCodec.BufferInfo();
            if(encoder == null) {
                LogUtil.e(TAG, "craete encoder wrong");
                return;
            }
            encoder.configure(encoderFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            outputStream = new FileOutputStream(outfile);
            encoder.start();
            LogUtil.d(TAG, "craete encoder success!");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void encodecPcmToAAc(int size, byte[] buffer) {

        double recordTime = size * 1.0 / (jniSamplerate() * 2 * (16 / 8));
        //LogUtil.d(TAG, jniSamplerate() + " " + size + " " + buffer.length + " recordTime:" + recordTime);
        if(buffer != null && encoder != null) {

            int inputBufferindex = encoder.dequeueInputBuffer(0);
//            LogUtil.d(TAG, "inputBufferindex " + inputBufferindex);
            if(inputBufferindex >= 0) {
                ByteBuffer byteBuffer = encoder.getInputBuffers()[inputBufferindex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                encoder.queueInputBuffer(inputBufferindex, 0, size, 0, 0);
            }

            int index = encoder.dequeueOutputBuffer(info, 0);
            while(index >= 0) {
                try {
                    perpcmsize = info.size + 7;
                    outByteBuffer = new byte[perpcmsize];

                    ByteBuffer byteBuffer = encoder.getOutputBuffers()[index];
                    byteBuffer.position(info.offset);
                    byteBuffer.limit(info.offset + info.size);

                    addADtsHeader(outByteBuffer, perpcmsize, aacsamplerate);

                    byteBuffer.get(outByteBuffer, 7, info.size);
                    byteBuffer.position(info.offset);
                    if (outputStream != null){
                        outputStream.write(outByteBuffer, 0, perpcmsize);
                    }

                    encoder.releaseOutputBuffer(index, false);
                    index = encoder.dequeueOutputBuffer(info, 0);
                    outByteBuffer = null;
//                    LogUtil.d(TAG,"编码...");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void addADtsHeader(byte[] packet, int packetLen, int samplerate) {
        int profile = 2; // AAC LC
        int freqIdx = samplerate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    private int getADTSsamplerate(int samplerate) {
        int rate = 4;
        switch (samplerate) {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }

    private void releaseAudioMediacodec() {
        if(encoder == null) {
            return;
        }
        try {
            outputStream.close();
            outputStream = null;
            encoder.stop();
            encoder.release();
            encoder = null;
            encoderFormat = null;
            info = null;
            initMediaCodec = false;

            LogUtil.d(TAG,"录制完成 release encoder success!");
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if(outputStream != null) {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }
    }



    public void setMGLSurfaceView(MGLSurfaceView surfaceView) {
        mglSurfaceView = surfaceView;
        LogUtil.d(TAG, "setMGLSurfaceView ! " + mglSurfaceView.getMRender());
        mglSurfaceView.getMRender().setOnSurfaceCreateListener(new MRender.OnSurfaceCreateListener() {
            @Override
            public void onSurfaceCreate(Surface surface) {
                if(mSurface == null) {
                    mSurface = surface;
                    LogUtil.d(TAG, "SurfaceCreateListener callback onSurfaceCreate!" + surface);
                }
            }
        });
    }

    private MediaFormat mediaFormat;
    private MediaCodec mediaCodec;
    private Surface mSurface;
    private MediaCodec.BufferInfo videoInfo;
    public void initMediaCodec(String codecName, int width, int height, byte[] csd_0, byte[] csd_1){
        if (mSurface != null){

            try {
                mglSurfaceView.getMRender().setRenderType(MRender.RENDER_MEDIACODEC);
                String mime = VideoSupportUtil.findVideoCodecName(codecName);
                LogUtil.d(TAG, "initMediaCodec mime=" + mime);

                mediaFormat = MediaFormat.createVideoFormat(mime, width, height);
                mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width*height);
                mediaFormat.setByteBuffer("csd_0", ByteBuffer.wrap(csd_0));
                mediaFormat.setByteBuffer("csd_1", ByteBuffer.wrap(csd_1));
                LogUtil.d(TAG, "mediaFormat=" + mediaFormat.toString());
                mediaCodec = MediaCodec.createDecoderByType(mime);

                videoInfo = new MediaCodec.BufferInfo();
                mediaCodec.configure(mediaFormat, mSurface, null, 0);
                mediaCodec.start();
                LogUtil.d(TAG, "initMediaCodec mediaCodec and start!");
            }catch (Exception e){
                LogUtil.e(TAG, e.toString());
            }
        } else {
            if(ffOnErrorListener != null) {
                ffOnErrorListener.onError(2001, "mSurface is null");
            }
        }

    }

    public void decodeAVPacket(int datasize, byte[] data){

        if(mSurface != null && datasize > 0 && data != null && mediaCodec != null){
            try {
                //设置解码等待时间，0为不等待，-1为一直等待，其余为时间单位
                int intputBufferIndex = mediaCodec.dequeueInputBuffer(10);
                if(intputBufferIndex >= 0) {
                    ByteBuffer byteBuffer = mediaCodec.getInputBuffers()[intputBufferIndex];
                    byteBuffer.clear();
                    byteBuffer.put(data);
                    //LogUtil.d(TAG, "decodeAVPacket data size=" + data.length);
                    mediaCodec.queueInputBuffer(intputBufferIndex, 0, datasize, 0, 0);
                }
                int outputBufferIndex = mediaCodec.dequeueOutputBuffer(videoInfo, 10);
                while(outputBufferIndex >= 0) {
                    mediaCodec.releaseOutputBuffer(outputBufferIndex, true);
                    outputBufferIndex = mediaCodec.dequeueOutputBuffer(videoInfo, 10);
                }
            }catch (Exception e){
                LogUtil.e(TAG, "decodeAVPacket :" + e.toString());
            }

        }else {
            LogUtil.e(TAG, "mSurface is null! " + (mSurface == null));
            LogUtil.e(TAG, "data is null! " + (data == null));
            LogUtil.e(TAG, "mediaCodec is null! " + (mediaCodec == null));
        }
    }

    private void releaseMediacodec() {
        if(mediaCodec != null) {
            LogUtil.d(TAG, "video decode start release!");
            try {
                mediaCodec.flush();
                mediaCodec.stop();
                mediaCodec.release();
            }catch (Exception e){
                LogUtil.e(TAG, "release Mediacodec exception " + e);
            }

            mediaCodec = null;
            mediaFormat = null;
            videoInfo = null;
        }
    }
}
