#include "../main.h"
#include "game.h"
#include "../util/arm.h"
#include "../gui/gui.h"
#include "../client/Multiplayer.h"
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <cstring>
#include <ctime>

static void MyAsciiToGxtChar(const char* src, uint16_t* dst, int maxLen)
{
    if (!src || !dst || maxLen <= 0) return;
    int i = 0;
    while (src[i] && i < maxLen - 1) {
        dst[i] = (uint16_t)(unsigned char)src[i];
        i++;
    }
    dst[i] = 0;
}

FindPlayerPed_t pfnFindPlayerPed = nullptr;
CWorld_Add_t pfnCWorld_Add = nullptr;
CWorld_Remove_t pfnCWorld_Remove = nullptr;
SetModelIndex_t pfnSetModelIndex = nullptr;
RequestModel_t pfnRequestModel = nullptr;
EntityTeleport_t pfnEntityTeleport = nullptr;

void* (*pfnGetIconForOptionOrig)(const char* key) = nullptr;
uint16_t* (*CTextGetOrig)(void* thiz, const char* text) = nullptr;

void (*Render2dStuffOrig)(void) = nullptr;
EGLBoolean (*eglSwapBuffersOrig)(EGLDisplay, EGLSurface) = nullptr;

static bool g_bInMainMenu = false;
static uint32_t g_lastMenuTick = 0;
static bool g_bImGuiFrameOpen = false;

static void* GetGameAddress(uintptr_t offset) {
#if defined(__arm__)
    return (void*)((gta_3_base + offset) | 1);
#else
    return (void*)(gta_3_base + offset);
#endif
}

static uint32_t GetTickMs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static void BeginImGuiFrame() {
    if (g_bImGuiFrameOpen) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    g_bImGuiFrameOpen = true;
}

static void EndImGuiFrame() {
    if (!g_bImGuiFrameOpen) return;
    ImGui::Render();
    ImDrawData* dd = ImGui::GetDrawData();
    if (dd) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ImGui_ImplOpenGL3_RenderDrawData(dd);
    }
    g_bImGuiFrameOpen = false;
}

void* GetIconForOptionHook(const char* key)
{
    if (key) {
        if (strcmp(key, "MP_PLAY") == 0) {
            return pfnGetIconForOptionOrig ? pfnGetIconForOptionOrig("FEP_STA") : nullptr;
        }

        if (strcmp(key, "FEP_STA") == 0 ||
            strcmp(key, "FEM_RES") == 0 ||
            strcmp(key, "FET_OPT") == 0 ||
            strcmp(key, "FEP_BRI") == 0 ||
            strcmp(key, "FEM_QT")  == 0 ||
            strcmp(key, "FEN_STA") == 0) {
            g_bInMainMenu = true;
            g_lastMenuTick = GetTickMs();
        }
    }
    return pfnGetIconForOptionOrig ? pfnGetIconForOptionOrig(key) : nullptr;
}

uint16_t* CTextGetHook(void* thiz, const char* text)
{
    if (!text) return CTextGetOrig(thiz, text);

    if (strcmp(text, "SPLASH") == 0) {
        UnFuck((void*)(gta_3_base + 0x65E404), sizeof(uint32_t));
        UnFuck((void*)(gta_3_base + 0x65E403), sizeof(uint32_t));
        uint32_t* legalScreenState = (uint32_t*)(gta_3_base + 0x65E404);
        uint32_t* shownLegalScreen = (uint32_t*)(gta_3_base + 0x65E403);
        *legalScreenState = 2;
        *shownLegalScreen = 1;
    }

    return CTextGetOrig(thiz, text);
}

void Render2dStuffHook(void)
{
    if (Render2dStuffOrig) Render2dStuffOrig();

    if (!pGUI) return;
    pGUI->Init();
    if (!pGUI->m_bInited) return;

    if (g_bInMainMenu) return;

    GLint vp[4] = {0};
    glGetIntegerv(GL_VIEWPORT, vp);
    if (vp[2] > 0 && vp[3] > 0) {
        ImGui::GetIO().DisplaySize = ImVec2((float)vp[2], (float)vp[3]);
    }

    BeginImGuiFrame();
    pGUI->Render();
    EndImGuiFrame();
}

EGLBoolean eglSwapBuffersHook(EGLDisplay dpy, EGLSurface surf)
{
    if (pGUI) {
        pGUI->Init();
        if (pGUI->m_bInited) {
            if (g_bInMainMenu && (GetTickMs() - g_lastMenuTick) > 500) {
                g_bInMainMenu = false;
            }

            if (g_bInMainMenu) {
                EGLint w = 0, h = 0;
                eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
                eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);
                if (w > 0 && h > 0) {
                    ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);
                }

                BeginImGuiFrame();
                pGUI->RenderMenuMultiplayerButton();
                EndImGuiFrame();
            }
        }
    }
    return eglSwapBuffersOrig(dpy, surf);
}

void* GetLocalPlayerPed() {
    if (!pfnFindPlayerPed && gta_3_base) {
        pfnFindPlayerPed = (FindPlayerPed_t)GetGameAddress(OFFSET_FindPlayerPed);
    }
    return pfnFindPlayerPed ? pfnFindPlayerPed(-1) : nullptr;
}

void WorldAddEntity(void* entity) {
    if (!entity) return;
    if (!pfnCWorld_Add && gta_3_base) {
        pfnCWorld_Add = (CWorld_Add_t)GetGameAddress(OFFSET_CWorld_Add);
    }
    if (pfnCWorld_Add) pfnCWorld_Add(entity);
}

void WorldRemoveEntity(void* entity) {
    if (!entity) return;
    if (!pfnCWorld_Remove && gta_3_base) {
        pfnCWorld_Remove = (CWorld_Remove_t)GetGameAddress(OFFSET_CWorld_Remove);
    }
    if (pfnCWorld_Remove) pfnCWorld_Remove(entity);
}

void EntitySetModelIndex(void* entity, unsigned int modelIndex) {
    if (!entity) return;
    if (!pfnSetModelIndex && gta_3_base) {
        pfnSetModelIndex = (SetModelIndex_t)GetGameAddress(OFFSET_SetModelIndex);
    }
    if (pfnSetModelIndex) pfnSetModelIndex(entity, modelIndex);
}

void RequestGameModel(int modelIndex, int flags) {
    if (!pfnRequestModel && gta_3_base) {
        pfnRequestModel = (RequestModel_t)GetGameAddress(OFFSET_RequestModel);
    }
    if (pfnRequestModel) pfnRequestModel(modelIndex, flags);
}

void EntityTeleport(void* entity, float x, float y, float z) {
    if (!entity) return;
    if (!pfnEntityTeleport && gta_3_base) {
        pfnEntityTeleport = (EntityTeleport_t)GetGameAddress(OFFSET_CEntity_Teleport);
    }
    if (pfnEntityTeleport) {
        Vector3Raw pos = { x, y, z };
        pfnEntityTeleport(entity, pos);
    }
}

void InstallCallHooks()
{
    LOGI("Installing Dobby Hooks for Android Multiplayer...");

    void* pGetIcon = GetGameAddress(OFFSET_GetIconForOption);
    if (pGetIcon) {
        DobbyHook(pGetIcon, (void*)GetIconForOptionHook, (void**)&pfnGetIconForOptionOrig);
        LOGI("GetIconForOption Hook installed at %p", pGetIcon);
    }

    void* pCTextGet = GetGameAddress(OFFSET_CText_Get);
    if (pCTextGet) {
        DobbyHook(pCTextGet, (void*)CTextGetHook, (void**)&CTextGetOrig);
        LOGI("CText::Get Hook installed at %p", pCTextGet);
    }

    void* pRender2D = GetGameAddress(OFFSET_Render2dStuff);
    if (pRender2D) {
        DobbyHook(pRender2D, (void*)Render2dStuffHook, (void**)&Render2dStuffOrig);
        LOGI("Render2DStuff Hook installed at %p", pRender2D);
    }

    void* eglSym = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
    if (eglSym) {
        DobbyHook(eglSym, (void*)eglSwapBuffersHook, (void**)&eglSwapBuffersOrig);
        LOGI("eglSwapBuffers Hook installed at %p", eglSym);
    } else {
        LOGE("eglSwapBuffers NOT found!");
    }

    LOGI("All hooks installed.");
}