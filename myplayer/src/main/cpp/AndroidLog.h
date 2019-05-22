#ifndef FFMUSIC_ANDROIDLOG_H  //先测试FFMUSIC_ANDROIDLOG_H是否被宏定义过
#define FFMUSIC_ANDROIDLOG_H

#endif //FFMUSIC_ANDROIDLOG_H


#include <android/log.h>

#define LOG_DEBUG true

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"wguter",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, "wguter",FORMAT,##__VA_ARGS__);