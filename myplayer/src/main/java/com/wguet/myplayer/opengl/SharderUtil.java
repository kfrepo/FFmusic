package com.wguet.myplayer.opengl;

import android.content.Context;
import android.opengl.GLES20;

import com.wguet.myplayer.util.LogUtil;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

/**
 * @author workw
 */
public class SharderUtil {

    private static final String TAG = "ShaderUtil";


    public static String readRawTxt(Context context, int rawId) {
        InputStream inputStream = context.getResources().openRawResource(rawId);
        BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
        StringBuffer sb = new StringBuffer();
        String line;
        try {
            while((line = reader.readLine()) != null) {
                sb.append(line).append("\n");
            }
            reader.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return sb.toString();
    }

    /**
     * 编译片段着色器
     * @param shaderType 着色器类型
     * @param source 编译代码
     * @return 着色器对象ID
     */
    public static int loadShader(int shaderType, String source) {
        // 1.创建一个新的着色器对象
        int shader = GLES20.glCreateShader(shaderType);
        if(shader != 0) {
            // 2.将着色器代码上传到着色器对象中
            GLES20.glShaderSource(shader, source);
            // 3.编译着色器对象
            GLES20.glCompileShader(shader);
            // 5.获取编译状态：OpenGL将想要获取的值放入长度为1的数组的首位
            int[] compile = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compile, 0);
            if(compile[0] != GLES20.GL_TRUE) {
                LogUtil.d(TAG, "shader compile error!");
                GLES20.glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    /**
     * 创建OpenGL程序对象
     * @param vertexSource 顶点着色器代码
     * @param fragmentSource 片段着色器代码
     * @return
     */
    public static int createProgram(String vertexSource, String fragmentSource) {
        //编译顶点着色器
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        if(vertexShader == 0) {
            return 0;
        }
        // 编译片段着色器
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        if(fragmentShader == 0) {
            return 0;
        }
        int program = GLES20.glCreateProgram();
        if(program != 0) {
            GLES20.glAttachShader(program, vertexShader);
            GLES20.glAttachShader(program, fragmentShader);
            //将顶点着色器、片段着色器进行链接，组装成一个OpenGL程序
            GLES20.glLinkProgram(program);
            int[] linsStatus = new int[1];
            GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linsStatus, 0);
            if(linsStatus[0] != GLES20.GL_TRUE) {
                LogUtil.d(TAG, "link program error");
                GLES20.glDeleteProgram(program);
                program = 0;
            }
        }
        return  program;
    }

}
