package com.wguet.myplayer.util;

import android.media.MediaCodecInfo;
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
//        codecMap.put("h264", "video/avc");
//        codecMap.put("h265", "video/hevc");
//        codecMap.put("vp9", "video/x-vnd.on2.vp9");
    }

    public static void init(){

        // 获取所有支持编解码器数量
        int count = MediaCodecList.getCodecCount();
        LogUtil.d("VideoSupportUtil", "支持编解码器数量 " + count);

        for (int i = 0; i < count; i++){
            // 编解码器相关性信息存储在MediaCodecInfo中
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
            if (codecInfo.isEncoder()){
                LogUtil.d("VideoSupportUtil", "Encoder " + Arrays.toString(codecInfo.getSupportedTypes()));
            }else {
                String[] types = codecInfo.getSupportedTypes();
                LogUtil.d("VideoSupportUtil", "Decoder " + Arrays.toString(types));
                for (int j = 0; j < types.length; j++){
                    if (types[j].equalsIgnoreCase("video/avc")){
                        codecMap.put("h264", "video/avc");
                    }
                    if (types[j].equalsIgnoreCase("video/hevc")){
                        codecMap.put("h265", "video/hevc");
                    }
                    if (types[j].equalsIgnoreCase("video/x-vnd.on2.vp9")){
//                        codecMap.put("vp9", "video/x-vnd.on2.vp9");
                    }
                }
            }
        }
    }

    public static String findVideoCodecName(String codeName){
        if (codecMap.containsKey(codeName)){
            return codecMap.get(codeName);
        }
        return "";
    }

    public static boolean isSupportCodec(String codecname){
        boolean supportvideo = false;
        // 获取所有支持编解码器数量
        int count = MediaCodecList.getCodecCount();
//        LogUtil.d("VideoSupportUtil", "支持编解码器数量 " + count);

        for (int i = 0; i < count; i++){
            // 编解码器相关性信息存储在MediaCodecInfo中
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
            if (codecInfo.isEncoder()){
//                LogUtil.d("VideoSupportUtil", "Encoder " + Arrays.toString(codecInfo.getSupportedTypes()));
            }else {
                String[] types = codecInfo.getSupportedTypes();
//                LogUtil.d("VideoSupportUtil", "Decoder " + Arrays.toString(types));
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
        }
        return supportvideo;
    }
}
