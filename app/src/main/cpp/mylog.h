//
// Created by cnting on 2023/9/14.
//

#ifndef OPENGLSTUDY_MYLOG_H
#define OPENGLSTUDY_MYLOG_H

#include <android/log.h>

#define TAG "===>"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#endif //OPENGLSTUDY_MYLOG_H
