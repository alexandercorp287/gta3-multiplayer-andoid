# Rebuilt for NDK 27: $(call my-dir) returns empty on this host because
# HOST_OS is not set yet when the path is evaluated. Use an explicit ROOT.
LOCAL_PATH := .
ROOT := C:/gta3-multiplayer-andoid-main/jni

include $(CLEAR_VARS)

LOCAL_MODULE := multiplayer
LOCAL_LDLIBS := -llog -lGLESv3 -lEGL -landroid

LOCAL_C_INCLUDES := $(ROOT) \
                    $(ROOT)/client \
                    $(ROOT)/game \
                    $(ROOT)/gui \
                    $(ROOT)/util \
                    $(ROOT)/vendor/Dobby \
                    $(ROOT)/vendor/imgui \
                    $(ROOT)/vendor/RakNet \
                    $(ROOT)/vendor/RakNet/SAMP

LOCAL_SRC_FILES := main.cpp \
                   client/CLocalPlayer.cpp \
                   client/CNetClient.cpp \
                   client/CPlayerManager.cpp \
                   client/CRemotePlayer.cpp \
                   client/Multiplayer.cpp \
                   client/keyboard.cpp \
                   game/hooks.cpp \
                   gui/CChatWindow.cpp \
                   gui/CDialogWindow.cpp \
                   gui/CSpawnScreen.cpp \
                   gui/gui.cpp \
                   gui/ServerList.cpp \
                   util/arm.cpp \
                   util/util.cpp \
                   vendor/imgui/imgui.cpp \
                   vendor/imgui/imgui_draw.cpp \
                   vendor/imgui/imgui_tables.cpp \
                   vendor/imgui/imgui_widgets.cpp \
                   vendor/imgui/imgui_impl_opengl3.cpp \
                   vendor/RakNet/AsynchronousFileIO.cpp \
                   vendor/RakNet/BitStream.cpp \
                   vendor/RakNet/BitStream_NoTemplate.cpp \
                   vendor/RakNet/CheckSum.cpp \
                   vendor/RakNet/CommandParserInterface.cpp \
                   vendor/RakNet/DataBlockEncryptor.cpp \
                   vendor/RakNet/DataCompressor.cpp \
                   vendor/RakNet/DS_ByteQueue.cpp \
                   vendor/RakNet/DS_HuffmanEncodingTree.cpp \
                   vendor/RakNet/DS_Table.cpp \
                   vendor/RakNet/FileOperations.cpp \
                   vendor/RakNet/GetTime.cpp \
                   vendor/RakNet/InternalPacketPool.cpp \
                   vendor/RakNet/LinuxStrings.cpp \
                   vendor/RakNet/NetworkTypes.cpp \
                   vendor/RakNet/PluginInterface.cpp \
                   vendor/RakNet/RakClient.cpp \
                   vendor/RakNet/RakNetStatistics.cpp \
                   vendor/RakNet/RakNetworkFactory.cpp \
                   vendor/RakNet/rakserver.cpp \
                   vendor/RakNet/RakPeer.cpp \
                   vendor/RakNet/RakSleep.cpp \
                   vendor/RakNet/Rand.cpp \
                   vendor/RakNet/ReliabilityLayer.cpp \
                   vendor/RakNet/rijndael.cpp \
                   vendor/RakNet/RPCMap.cpp \
                   vendor/RakNet/SHA1.cpp \
                   vendor/RakNet/SimpleMutex.cpp \
                   vendor/RakNet/SocketLayer.cpp \
                   vendor/RakNet/StringCompressor.cpp \
                   vendor/RakNet/StringTable.cpp \
                   vendor/RakNet/SystemAddressList.cpp \
                   vendor/RakNet/TableSerializer.cpp \
                   vendor/RakNet/TCPInterface.cpp \
                   vendor/RakNet/SAMP/SAMPRPC.cpp \
                   vendor/RakNet/SAMP/samp_auth.cpp \
                   vendor/RakNet/SAMP/samp_netencr.cpp

LOCAL_CPPFLAGS := -D_ARM_ -w -pthread -O2 -std=c++11 -DRAKSAMP_CLIENT -D_ANDROID -D__ANDROID__

LOCAL_LDFLAGS := -Wl,-z,now -Wl,-z,relro -Wl,-z,noexecstack

LOCAL_STATIC_LIBRARIES := dobby_prebuilt

include $(BUILD_SHARED_LIBRARY)

# --- Dobby ---
include $(CLEAR_VARS)
LOCAL_MODULE := dobby_prebuilt
LOCAL_SRC_FILES := $(patsubst ./%,%,$(ROOT)/vendor/Dobby/$(TARGET_ARCH_ABI)/libdobby.a)
LOCAL_EXPORT_C_INCLUDES := $(ROOT)/vendor/Dobby
include $(PREBUILT_STATIC_LIBRARY)
