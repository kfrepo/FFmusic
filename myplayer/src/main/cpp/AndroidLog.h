#ifndef FFMUSIC_ANDROIDLOG_H  //先测试FFMUSIC_ANDROIDLOG_H是否被宏定义过
#define FFMUSIC_ANDROIDLOG_H

#endif //FFMUSIC_ANDROIDLOG_H


#include <android/log.h>

#define LOG_DEBUG true

#define LOGI(FORMAT,...) if(LOG_DEBUG)__android_log_print(ANDROID_LOG_INFO, "native",FORMAT, ##__VA_ARGS__);
#define LOGE(FORMAT,...) if(LOG_DEBUG)__android_log_print(ANDROID_LOG_ERROR, "native",FORMAT, ##__VA_ARGS__);

#define LOGIT(TAG, FORMAT, ...) if(LOG_DEBUG)__android_log_print(ANDROID_LOG_INFO, TAG, FORMAT, ##__VA_ARGS__);