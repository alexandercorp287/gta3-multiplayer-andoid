#include "main.h"
#include "client/Multiplayer.h"
#include "vendor/imgui/imgui.h"

void InstallCallHooks();
//void PatchMainMenu();   // ← объявление

extern "C" JNIEXPORT void JNICALL
Java_com_rockstar_gta3_GTA3_startMultiplayer(JNIEnv* env, jclass clazz) {
    LOGI("JNI: startMultiplayer invoked!");
    CMultiplayer::bWantMP = true;
    CMultiplayer::Initialise();
}

extern "C" JNIEXPORT void JNICALL
Java_com_rockstar_gta3_GTA3_sendTouchEvent(JNIEnv* env, jclass clazz, jint action, jfloat x, jfloat y) {
    if (ImGui::GetCurrentContext() == nullptr) return;
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);

    if (action == 0 || action == 2) {
        io.MouseDown[0] = true;
    } else if (action == 1 || action == 3) {
        io.MouseDown[0] = false;
    }
}

void* InitThread(void* arg)
{
    LOGI("InitThread started, waiting for libR1.so...");

    // Ждём загрузки libR1.so (вместо sleep — polling)
    for (int i = 0; i < 100 && !gta_3_base; i++) {
        gta_3_base = find_library("libR1.so");
        if (!gta_3_base) usleep(100000); // 100 мс
    }

    if (!gta_3_base) {
        LOGE("libR1.so not found in InitThread!");
        return nullptr;
    }
    LOGI("libR1.so base: 0x%lX", (unsigned long)gta_3_base);

    // 1. Установка хуков (GetIconForOption, CText::Get, ProcessButtonPresses)
    InstallCallHooks();

    LOGI("InitThread: done");
    return nullptr;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOGI("libmultiplayer.so loaded!");

    // gta_3_base найдём в InitThread — в JNI_OnLoad libR1.so может быть ещё не загружена
    pthread_t thread;
    pthread_create(&thread, nullptr, InitThread, nullptr);
    pthread_detach(thread);

    return JNI_VERSION_1_6;
}