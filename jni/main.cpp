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

    // Координаты из MotionEvent.getX/getY идут в пикселях экрана.
    // Если DisplaySize отличается (например, задан из GL viewport другого
    // размера), масштабируем — иначе нажатия не попадают по виджетам.
    float mx = x, my = y;
    if (io.DisplaySize.x > 0.0f && io.DisplaySize.y > 0.0f) {
        static float lastW = 0.0f, lastH = 0.0f;
        static float screenW = 0.0f, screenH = 0.0f;

        // Размер окна в пикселях экрана (в тех же единицах, что getX/getY)
        if (screenW <= 0.0f) {
            // Запрашиваем один раз через ANativeWindow, но проще — эвристика:
            // считаем, что экран больше DisplaySize или равен ему.
            screenW = io.DisplaySize.x;
            screenH = io.DisplaySize.y;
        }
        if (lastW != io.DisplaySize.x || lastH != io.DisplaySize.y) {
            lastW = io.DisplaySize.x;
            lastH = io.DisplaySize.y;
        }

        // Если координаты явно выходят за DisplaySize — масштабируем пропорционально.
        if (x > io.DisplaySize.x + 1.0f || y > io.DisplaySize.y + 1.0f) {
            if (screenW > 0.0f && screenH > 0.0f) {
                mx = x * (io.DisplaySize.x / screenW);
                my = y * (io.DisplaySize.y / screenH);
            }
        }
    }

    io.MousePos = ImVec2(mx, my);

    // MotionEvent action (уже getActionMasked из smali):
    //   0 DOWN, 1 UP, 2 MOVE, 3 CANCEL, 5 POINTER_DOWN, 6 POINTER_UP
    switch (action) {
        case 0:   // ACTION_DOWN
            io.MouseDown[0] = true;
            break;

        case 2:   // ACTION_MOVE — кнопка остаётся нажатой
            break;

        case 1:   // ACTION_UP
            io.MouseDown[0] = false;
            // Палец отпущен — считаем это тапом по меню
            g_menuTapToken++;
            break;

        case 3:   // ACTION_CANCEL
        case 6:   // ACTION_POINTER_UP
            io.MouseDown[0] = false;
            break;

        case 5:   // ACTION_POINTER_DOWN — второй палец игнорируем
        default:
            break;
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