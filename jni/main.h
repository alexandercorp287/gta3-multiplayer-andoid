#pragma once

#include <jni.h>
#include <android/log.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>

#include "dobby.h"

#define LOG_TAG "RE3_MP_ANDROID"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern uintptr_t gta_3_base;

// Тапы по экрану меню (увеличивается при ACTION_UP в sendTouchEvent)
extern int  g_menuTapToken;
// Открыть окно списка серверов (вместо прямого подключения)
extern bool g_bShowServerList;
// Запуск игры из меню (ставится хуком в hooks.cpp)
extern void (*DoSettingsBeforeStartingAGame)();

uintptr_t find_library(const char* lib);
