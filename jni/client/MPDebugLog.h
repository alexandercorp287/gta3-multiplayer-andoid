#pragma once

#include <android/log.h>
#include "Re3Log.h"

// MP debug logging. Disabled by default (release perf).
// Enable with -DMP_DEBUG_LOG in the build (e.g. for logcat debugging).
#if defined(MP_DEBUG_LOG)
#define LOG_TAG_MP "RE3_MP"
#define LOGI(...) RE3_LOGI(LOG_TAG_MP, __VA_ARGS__)
#define LOGE(...) RE3_LOGE(LOG_TAG_MP, __VA_ARGS__)
#define LOGSYNC(...) RE3_LOGI("sync_ped", __VA_ARGS__)
#else
#define LOGI(...) ((void)0)
#define LOGE(...) ((void)0)
#define LOGSYNC(...) ((void)0)
#endif
