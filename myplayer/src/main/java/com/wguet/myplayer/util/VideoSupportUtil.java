package com.wguet.myplayer.util;

import android.media.MediaCodecList;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * Description :
 *
 * @author wangmh
 * @date 2020/12/10 19:25
 */
public class VideoSupportUtil {

    private static Map<String, String> codecMap = new HashMap<>();
    static {
        codecMap.put("h264", "video/avc");
    }

    public static String findVideoCodecName(String codeName){
        if (codecMap.containsKey(codeName)){
            return codecMap.get(codeName);
        }
        return "";
    }

    public static boolean isSupportCodec(String codecname){
        boolean supportvideo = false;
        int count = MediaCodecList.getCodecCount();
        for (int i = 0; i < count; i++){
            String[] types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            LogUtil.d("VideoSupportUtil", Arrays.toString(types));
            for (int j = 0; j < types.length; j++){
                if (types[j].equals(findVideoCodecName(codecname))){
                    supportvideo = true;
                    break;
                }
            }
            if (supportvideo){
                break;
            }
        }
        return supportvideo;
    }
}
