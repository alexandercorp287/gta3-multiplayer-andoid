// The logs of the game.
//
// They are only useful when something goes wrong (a crash, a missing model,
// a wrong frame time), and writing to the logcat costs a syscall and a
// format on every call, so they are compiled out in the release build.
// The errors are always logged: they are the reason for the logcat.
#ifndef __GTA_RE3LOG_H__
#define __GTA_RE3LOG_H__

#if defined(ANDROID)
#include <android/log.h>
#define RE3_LOGE(tag, ...) __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
#define RE3_LOGW(tag, ...) __android_log_print(ANDROID_LOG_WARN, tag, __VA_ARGS__)
#if !defined(NDEBUG) || defined(RE3_DIAGNOSTICS)
#define RE3_LOGI(tag, ...) __android_log_print(ANDROID_LOG_INFO, tag, __VA_ARGS__)
#else
#define RE3_LOGI(tag, ...) ((void)0)
#endif
#else
#define RE3_LOGE(tag, ...) ((void)0)
#define RE3_LOGW(tag, ...) ((void)0)
#define RE3_LOGI(tag, ...) ((void)0)
#endif

#endif // __GTA_RE3LOG_H__
