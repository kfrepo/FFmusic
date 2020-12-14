package com.wguet.myplayer.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.wguet.myplayer.util.LogUtil;


/**
 * Description :
 *
 * @author wangmh
 * @date 2020/12/7 20:02
 */
public class MGLSurfaceView extends GLSurfaceView {

    private MRender mRender;

    public MGLSurfaceView(Context context) {
        this(context, null);
    }

    public MGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        mRender = new MRender(context);
        setRenderer(mRender);
        //设置渲染方式 只有在创建和调用requestRender()时才会刷新
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        LogUtil.d("MGLSurfaceView", "MGLSurfaceView constructed!");

        mRender.setOnRenderListener(new MRender.OnRenderListener() {
            @Override
            public void onRender() {
                requestRender();
            }
        });
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v) {
        if(mRender != null) {
            mRender.setYUVRenderData(width, height, y, u, v);
            //通过requestRender()方法主动请求重绘
            requestRender();
        }
    }

    public MRender getMRender() {
        return mRender;
    }
}
