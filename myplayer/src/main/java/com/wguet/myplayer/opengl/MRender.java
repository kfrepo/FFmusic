package com.wguet.myplayer.opengl;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.wguet.myplayer.R;
import com.wguet.myplayer.util.LogUtil;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


/**
 * Description :
 *
 * @author wangmh
 * @date 2020/12/7 20:04
 */
public class MRender implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener{

    private final static String TAG = "MRender";

    public static final int RENDER_YUV = 1;
    public static final int RENDER_MEDIACODEC = 2;

    private Context context;

    /**
     * 绘制坐标范围
     */
    private final float[] vertexData = {
            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f
    };

    private final float[] textureData ={
            0f,1f,
            1f, 1f,
            0f, 0f,
            1f, 0f
    };

    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;

    private int renderType = RENDER_YUV;

    //YUV
    private int program_yuv;
    private int avPosition_yuv;
    private int afPosition_yuv;

    private int sampler_y;
    private int sampler_u;
    private int sampler_v;
    private int[] textureId_yuv;

    private int width_yuv;
    private int height_yuv;
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;

    //mediacodec
    private int program_mediacodec;
    private int avPosition_mediacodec;
    private int afPosition_mediacodec;
    private int samplerOES_mediacodec;
    private int textureId_mediacodec;
    private SurfaceTexture surfaceTexture;
    private Surface surface;

    private OnSurfaceCreateListener onSurfaceCreateListener;
    private OnRenderListener onRenderListener;

    public MRender(Context context){
        this.context = context;
        //为坐标分配本地内存地址
        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                // 将Java Dalvik的内存数据复制到Native内存中
                .put(vertexData);
        // 将缓冲区的指针移动到头部，保证数据是从最开始处读取
        vertexBuffer.position(0);

        textureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureData);
        textureBuffer.position(0);
        LogUtil.d(TAG, "MRender constructed!");
    }


    public void setRenderType(int renderType) {
        this.renderType = renderType;
    }

    public void setOnSurfaceCreateListener(OnSurfaceCreateListener onSurfaceCreateListener) {
        this.onSurfaceCreateListener = onSurfaceCreateListener;
    }

    public void setOnRenderListener(OnRenderListener onRenderListener) {
        this.onRenderListener = onRenderListener;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        LogUtil.d(TAG, "onSurfaceCreated");
        initRenderYUV();
        initRenderMediacodec();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        LogUtil.d(TAG, "onSurfaceChanged " + width + "x" + height);
        //x，y 以像素为单位，指定了视口的左下角位置。
        //width，height 表示这个视口矩形的宽度和高度，根据窗口的实时变化重绘窗口。
        GLES20.glViewport( 0, 0, width, height);
    }

    /**
     * Buffer数据在传递给GLSL之前，一定要调用position方法将指针移到正确的位置
     * 一定要先上色，再绘制图形，否则会导致颜色在当前这一帧使用失败，要下一帧才能生效。
     * @param gl
     */
    @Override
    public void onDrawFrame(GL10 gl) {

        //指定刷新颜色缓冲区时所用的颜色,清除颜色缓冲区的作用是，
        GLES20.glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //清除颜色缓冲,防止缓冲区中原有的颜色信息影响本次绘图
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        if (renderType == RENDER_YUV) {
            renderYUV();
        }else if (renderType == RENDER_MEDIACODEC) {
            renderMediacodec();
        }
        // 图形绘制
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        //新的数据帧有效时的回调接口
        if(onRenderListener != null) {
            //回调requestRender() 主动请求重绘
            onRenderListener.onRender();
        }
    }

    private void initRenderYUV() {
        LogUtil.d(TAG, "initRenderYUV!");
        String vertexSource = SharderUtil.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = SharderUtil.readRawTxt(context, R.raw.fragment_yuv);
        program_yuv = SharderUtil.createProgram(vertexSource, fragmentSource);

        //获取着色器程序中，指定为attribute类型变量的id。
        avPosition_yuv = GLES20.glGetAttribLocation(program_yuv, "av_Position");
        afPosition_yuv = GLES20.glGetAttribLocation(program_yuv, "af_Position");

        //获取着色器程序中，指定为uniform类型变量的id。
        sampler_y = GLES20.glGetUniformLocation(program_yuv, "sampler_y");
        sampler_u = GLES20.glGetUniformLocation(program_yuv, "sampler_u");
        sampler_v = GLES20.glGetUniformLocation(program_yuv, "sampler_v");

        textureId_yuv = new int[3];
        GLES20.glGenTextures(3, textureId_yuv, 0);

        for(int i = 0; i < 3; i++) {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[i]);

            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
    }

    public void setYUVRenderData(int width, int height, byte[] y, byte[] u, byte[] v) {
        this.width_yuv = width;
        this.height_yuv = height;
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

    private void renderYUV() {

        if(width_yuv > 0 && height_yuv > 0 && y != null && u != null && v != null) {
            GLES20.glUseProgram(program_yuv);

            GLES20.glEnableVertexAttribArray(avPosition_yuv);
            GLES20.glVertexAttribPointer(avPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);

            GLES20.glEnableVertexAttribArray(afPosition_yuv);
            GLES20.glVertexAttribPointer(afPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

            // 绑定纹理
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[0]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv, height_yuv, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v);

            GLES20.glUniform1i(sampler_y, 0);
            GLES20.glUniform1i(sampler_u, 1);
            GLES20.glUniform1i(sampler_v, 2);

            y.clear();
            u.clear();
            v.clear();
            y = null;
            u = null;
            v = null;
        }
    }

    private void initRenderMediacodec() {
        LogUtil.d(TAG, "initRenderMediacodec!");
        String vertexSource = SharderUtil.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = SharderUtil.readRawTxt(context, R.raw.fragment_mediacodec);
        program_mediacodec = SharderUtil.createProgram(vertexSource, fragmentSource);

        avPosition_mediacodec = GLES20.glGetAttribLocation(program_mediacodec, "av_Position");
        afPosition_mediacodec = GLES20.glGetAttribLocation(program_mediacodec, "af_Position");
        samplerOES_mediacodec = GLES20.glGetUniformLocation(program_mediacodec, "sTexture");

        int[] textureids = new int[1];
        GLES20.glGenTextures(1, textureids, 0);
        textureId_mediacodec = textureids[0];

        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        surfaceTexture = new SurfaceTexture(textureId_mediacodec);
        surface = new Surface(surfaceTexture);
        //从Image Stream中捕获帧数据，用作OpenGLES的纹理，其中Image Stream来自相机预览或视频解码
        surfaceTexture.setOnFrameAvailableListener(this);

        if(onSurfaceCreateListener != null) {
            onSurfaceCreateListener.onSurfaceCreate(surface);
        }
    }

    private void renderMediacodec() {
        //当updateTexImage()被调用时，SurfaceTexture对象所关联的OpenGLES中纹理对象的内容将被更新为Image Stream中最新的图片。
        //SurfaceTexture对象可以在任何线程中创建。updateTexImage()方法只能在包OpenGLES环境的线程里调用，
        // 即Renderer接口所独立创建的线程当中。一般在onDrawFrame中调用updateTexImage()将数据绑定给OpenGLES对应的纹理对象。
        surfaceTexture.updateTexImage();
        GLES20.glUseProgram(program_mediacodec);

        GLES20.glEnableVertexAttribArray(avPosition_mediacodec);
        GLES20.glVertexAttribPointer(avPosition_mediacodec, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);

        GLES20.glEnableVertexAttribArray(afPosition_mediacodec);
        GLES20.glVertexAttribPointer(afPosition_mediacodec, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId_mediacodec);
        GLES20.glUniform1i(samplerOES_mediacodec, 0);
    }

    public interface OnSurfaceCreateListener {
        void onSurfaceCreate(Surface surface);
    }

    public interface OnRenderListener{
        void onRender();
    }
}
