package com.wguet.myplayer.util;

/**
 * @author workw
 */
public class TimeUtil {

    public static String secondsToDateFormat(int seconds, int totalseconds) {
        long hour = seconds / (60 * 60);
        long minute = (seconds % (60 * 60)) / (60);
        long second = seconds % (60);

        String strHour = "00";
        if (hour > 0) {
            if (hour < 10) {
                strHour = "0" + hour;
            } else {
                strHour = hour + "";
            }
        }

        String strMinute = "00";
        if (minute > 0) {
            if (minute < 10) {
                strMinute = "0" + minute;
            } else {
                strMinute = minute + "";
            }
        }

        String strSecond = "00";
        if (second > 0) {
            if (second < 10) {
                strSecond = "0" + second;
            } else {
                strSecond = second + "";
            }
        }
        if (totalseconds >= 3600) {
            return strHour + ":" + strMinute + ":" + strSecond;
        }
        return strMinute + ":" + strSecond;
    }
}
