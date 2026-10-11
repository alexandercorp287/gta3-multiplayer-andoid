LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := multiplayer
LOCAL_LDLIBS := -llog -lGLESv3 -lEGL -landroid

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/client \
                    $(LOCAL_PATH)/game \
                    $(LOCAL_PATH)/gui \
                    $(LOCAL_PATH)/util \
                    $(LOCAL_PATH)/vendor/Dobby \
                    $(LOCAL_PATH)/vendor/imgui \
                    $(LOCAL_PATH)/vendor/RakNet

LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/client/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/game/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/gui/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/util/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/vendor/imgui/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/vendor/RakNet/*.cpp)

LOCAL_CPPFLAGS := -D_ARM_ -w -pthread -O3 -fvisibility=hidden -std=c++11 -DRAKSAMP_CLIENT -D_ANDROID -D__ANDROID__

LOCAL_LDFLAGS := -Wl,-z,now -Wl,-z,relro -Wl,-z,noexecstack -Wl,-s

LOCAL_STATIC_LIBRARIES := dobby_prebuilt

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := dobby_prebuilt
LOCAL_SRC_FILES := vendor/Dobby/$(TARGET_ARCH_ABI)/libdobby.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/vendor/Dobby
include $(PREBUILT_STATIC_LIBRARY)
