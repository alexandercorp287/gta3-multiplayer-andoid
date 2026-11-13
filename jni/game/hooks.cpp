#include "../main.h"
#include "game.h"
#include "../util/arm.h"
#include "../gui/gui.h"
#include "../gui/ServerList.h"
#include "../client/Multiplayer.h"
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <cstring>
#include <ctime>

// =====================================================================
//  Главное меню: настоящий пункт MULTIPLAYER (подход как в build69)
//
//  В GTA:SA build69 добавляет новый пункт через OsArray<MenuItem>::Add.
//  В GTA III такой структуры нет — меню строится из статической таблицы
//  aScreens и текстов GXT. Поэтому делаем так:
//    1. CText::Get        — подменяем текст пункта на "MULTIPLAYER"
//    2. GetIconForOption  — подменяем иконку пункта
//    3. ProcessButtonPresses — ловим выбор этого пункта
// =====================================================================

// Пункт меню, который мы переиспользуем под MULTIPLAYER.
// "FEP_STG" = Start Game в главном меню GTA III.
static const char* MP_SOURCE_ITEM = "FEN_STA";
static const char* MP_LABEL       = "MULTIPLAYER";

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

// Счётчик тапов по экрану меню. Увеличивается в sendTouchEvent при ACTION_UP,
// поэтому выбор пункта реагирует на нажатие, а не на прокрутку.
int g_menuTapToken = 0;
// true — открыть окно списка серверов вместо прямого коннекта
bool g_bShowServerList = false;

// Читает main_mp.scm в память (реализация ниже)
static bool LoadMpScriptToMemory();
static uint32_t g_lastMenuTick = 0;
static bool g_bImGuiFrameOpen = false;
static bool g_bMenuPatched = false;

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

// ---------------------------------------------------------------------
//  1. Иконка пункта
// ---------------------------------------------------------------------
void* GetIconForOptionHook(const char* key)
{
    if (key) {
        // Наш пункт — берём иконку от Start Game
        if (strcmp(key, "MP_STA") == 0) {
            void* icon = pfnGetIconForOptionOrig ? pfnGetIconForOptionOrig("FEN_STA") : nullptr;
            if (!icon && pfnGetIconForOptionOrig) icon = pfnGetIconForOptionOrig("FEP_STA");
            if (!icon && pfnGetIconForOptionOrig) icon = pfnGetIconForOptionOrig("FET_OPT");
            LOGI("GetIconForOption(MP_STA) -> %p", icon);
            return icon;
        }

        if (strcmp(key, "FEN_STA") == 0 || strcmp(key, "FET_OPT") == 0 ||
            strcmp(key, "FEM_QT")  == 0) {
            g_bInMainMenu = true;
            g_lastMenuTick = GetTickMs();
        }
    }
    return pfnGetIconForOptionOrig ? pfnGetIconForOptionOrig(key) : nullptr;
}

// ---------------------------------------------------------------------
//  2. НАСТОЯЩИЙ пункт меню: вставляем слот в таблицу меню
//
//  По дизассемблеру CMenuManager::ProcessButtonPresses (0x22C08C):
//     menuTable = *(void**)(base + 0x46DE68)
//     screen    = menuTable + screenIdx * 0x19C
//     item      = screen    + itemIdx   * 0x14
//     action    = *(uint32_t*)(item + 0x34)
//
//  Записи с action == 0 игра пропускает (заворачивает на 0), поэтому
//  мы просто занимаем первый пустой слот главного экрана и пишем туда
//  свой action + свой текст.
// ---------------------------------------------------------------------
static uint16_t g_mpText[160];
static char*   g_mpSlotText = nullptr;   // текст, который увидит игра
static void*   g_mpSlot     = nullptr;
static int     g_activeScreen = -1;
static int     g_menuScreenIdx = 51;
static bool    g_mpInserted = false;

// Дамп таблицы меню, чтобы понять реальные смещения
// Формула из дизассемблера DrawFrontEndNormal (0x22397c) и
// ProcessButtonPresses (0x22c96c):  screen = aScreens + idx*0x19C
#define MENU_ENTRY_BASE   0x48
#define MENU_ENTRY_STRIDE 0x14
#define MENU_ENTRY_TEXT   0x00

// Главное меню GTA III = экран 52 (подтверждено дампом):
//   [0] FEN_STA  [1] FEP_STA  [2] FEP_BRI  [3] FET_OPT  [4] FEM_QT  [5] свободно
#define MP_SCREEN_INDEX   51
#define MP_ENTRY_SLOT     5

static void DumpMenuTable(void* table, int curScreen) {}

// ---------------------------------------------------------------------
//  Дорисовка пункта MULTIPLAYER под списком главного меню
//  (как build69 — свой пункт, но нарисованный вручную)
// ---------------------------------------------------------------------
typedef void (*PrintString_t)(float x, float y, uint16_t* text);
static PrintString_t pfnPrintString = nullptr;

static int  g_menuCount    = 0;   // сколько пунктов в текущем экране
static bool g_inMainMenu   = false;

static void TryInsertMultiplayerItem() {}

// ---------------------------------------------------------------------
//  Дорисовка пункта MULTIPLAYER (вариант B)
//
//  Координаты пунктов главного меню (из лога, 1570x720):
//     START GAME  x=750.0   y=635.8
//     OPTIONS     x=958.8   y=564.3
//     QUIT GAME   x=1139.6  y=490.9
//  Шаг: x += 208.8, y -= 70.9  =>  4-й пункт: x=1320.4, y=417.1
// ---------------------------------------------------------------------
typedef void (*PrintString_t)(float x, float y, uint16_t* text);
static PrintString_t pfnPrintStringOrig = nullptr;

#define MP_ITEM_X   1320.4f
#define MP_ITEM_Y    417.1f

static void PrintStringHook(float x, float y, uint16_t* text)
{
    if (pfnPrintStringOrig) pfnPrintStringOrig(x, y, text);
}

// ---------------------------------------------------------------------
//  Список серверов — рисуется тем же CFont::PrintString, что и пункт меню.
//  ImGui в главном меню не работает (кадр не начинается), поэтому всё
//  рисуем средствами игры.
// ---------------------------------------------------------------------
struct MpServer { const char* ip; int port; const char* name; };
static const MpServer kMpServers[] = {
    { "127.0.0.1",      5555, "Local server" },
    { "192.168.1.1",    5555, "Home LAN"     },
};
static const int kMpServerCount = (int)(sizeof(kMpServers)/sizeof(kMpServers[0]));
static int g_mpSelectedServer = 0;

static void DrawServerList()
{
    // Список серверов отключён: подключение идёт сразу по адресу из конфига.
}

// Вставляем наш пункт в ПЕРВЫЙ СВОБОДНЫЙ слот экранов 48..56.
// Какой из них рисуется — покажет лог; пункт появится именно там.
static void InsertMultiplayerInto(int idx, void* table)
{
    uint8_t* sc = (uint8_t*)table + idx * MENU_SCREEN_STRIDE;
    for (int it = 0; it < 12; it++) {
        uint8_t* e = sc + MENU_ENTRY_BASE + it * MENU_ENTRY_STRIDE;
        if (*(uint32_t*)(e + 0) != 0) continue;
        *(uint32_t*)(e + 0) = MENU_ACTION_STARTMULTIPLAYER;
        memcpy(e + 4, "MP_STA", 6);
        g_mpSlot = e;
        LOGI("MENU: MULTIPLAYER -> screen %d slot %d", idx, it);
        return;
    }
}

static void DrawMultiplayerItem(void* mgr)
{
    if (g_mpInserted || !gta_3_base) return;
    void** ppTable = (void**)(gta_3_base + OFFSET_g_MenuTablePtr);
    void*  table   = *ppTable;
    if (!table) return;

    // индексы, среди которых лежит главное меню
    static const int idxs[] = { 51, 52 };
    for (int k = 0; k < (int)(sizeof(idxs)/sizeof(idxs[0])); k++)
        InsertMultiplayerInto(idxs[k], table);

    g_mpInserted = true;
    LOGI("MENU: MULTIPLAYER inserted into screens 50..56");
}

// Текст: если игра просит наш ключ — отдаём "MULTIPLAYER"
uint16_t* CTextGetHook(void* thiz, const char* text)
{
    if (!CTextGetOrig) return nullptr;

    // Ставим пункт при первом же обращении игры к текстам меню
    TryInsertMultiplayerItem();

    if (!text) return CTextGetOrig(thiz, text);

    // DEBUG: логируем запрашиваемые тексты меню
    {
        static char seen[24][24];
        static int nseen = 0;
        bool dup = false;
        for (int k = 0; k < nseen; k++) if (strcmp(seen[k], text) == 0) { dup = true; break; }
        if (!dup && nseen < 24) {
            strncpy(seen[nseen], text, 23); seen[nseen][23] = 0; nseen++;
            LOGI("CText::Get requested: %s", text);
        }
    }

    // Главное меню GTA III запрашивает ровно эти три текста
    if (strcmp(text, "FEN_STA") == 0 || strcmp(text, "FET_OPT") == 0 ||
        strcmp(text, "FEM_QT")  == 0) {
        if (!g_bInMainMenu) LOGI("CText::Get: -> MAIN MENU detected");
        g_bInMainMenu = true;
        g_lastMenuTick = GetTickMs();
    }

    if (strcmp(text, "MP_STA") == 0) {
        MyAsciiToGxtChar(MP_LABEL, g_mpText, 64);
        return g_mpText;
    }

    return CTextGetOrig(thiz, text);
}

// ---------------------------------------------------------------------
//  3. Отрисовка (без ImGui-кнопки — пункт теперь настоящий)
// ---------------------------------------------------------------------
void Render2dStuffHook(void)
{
    if (Render2dStuffOrig) Render2dStuffOrig();

    // Главный тик мультиплеера: сеть, игроки, спавн
    if (CMultiplayer::IsActive()) {
        CMultiplayer::Process();
    }

    // Список серверов — тоже здесь, не завязан на pGUI/ImGui
    DrawServerList();

    // Всё ниже — ImGui, а он живёт только после Initialise()
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

            // ImGui-кнопка в меню больше не нужна: пункт настоящий.
            // Рисуем только если игра ещё не в меню (заставка/загрузка).
            if (!g_bInMainMenu && !CMultiplayer::IsActive()) {
                EGLint w = 0, h = 0;
                eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
                eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);
                if (w > 0 && h > 0) {
                    ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);
                }
            }
        }
    }
    return eglSwapBuffersOrig(dpy, surf);
}

// ---------------------------------------------------------------------
//  4. Нажатие пункта: вместо одиночной игры — мультиплеер
//
//  CMenuManager::DoSettingsBeforeStartingAGame вызывается, когда игрок
//  выбрал "Start Game". Мы переименовали этот пункт в MULTIPLAYER,
//  поэтому по нажатию поднимаем мультиплеер, а затем пускаем игру дальше.
// ---------------------------------------------------------------------
void (*DoSettingsBeforeStartingAGameOrig)() = nullptr;

// Для других модулей (окно списка серверов): запуск игры из меню
void (*DoSettingsBeforeStartingAGame)() = nullptr;
void (*ProcessButtonPressesOrig)(void*) = nullptr;

// Перехват выбора пункта (пока только безопасное наблюдение)
void ProcessButtonPressesHook(void* this_ptr)
{
    if (this_ptr) {
        g_activeScreen = *(int32_t*)((uint8_t*)this_ptr + 0x7CC);
    }
    TryInsertMultiplayerItem();

    if (ProcessButtonPressesOrig) ProcessButtonPressesOrig(this_ptr);

    if (g_activeScreen >= 48) DrawMultiplayerItem(nullptr);

    // Список серверов рисуем отсюда: ProcessButtonPresses точно идёт каждый
    // кадр меню, в отличие от Render2dStuff (он не зовётся в главном меню).
    DrawServerList();

    if (this_ptr) {
        static int lastSel = -1;
        static int lastScr = -1;
        uint8_t* mgr = (uint8_t*)this_ptr;
        int32_t sel = *(int32_t*)(mgr + 0x7D0);
        int32_t scr = *(int32_t*)(mgr + 0x7CC);
        g_activeScreen = scr;
        if (sel != lastSel || scr != lastScr) {
            lastSel = sel; lastScr = scr;
            LOGI("MENU state: screen=%d selected=%d", scr, sel);
        }

        // Наш пункт MULTIPLAYER. Подтверждение — только по тапу,
        // чтобы пункт не срабатывал при прокрутке списка.
        if (scr == MENU_SCREEN_MAIN) {
            static int lastTapToken = -1;
            if (sel == MENU_SLOT_MULTIPLAYER && g_menuTapToken != lastTapToken) {
                LOGI("MENU: MULTIPLAYER tapped -> starting MP");
                lastTapToken = g_menuTapToken;

                // Без сервер-листа: сразу подключаемся по адресу из конфига
                CMultiplayer::bWantMP = true;
                CMultiplayer::Initialise();

                if (DoSettingsBeforeStartingAGame) {
                    LOGI("MENU: -> loading game");
                    DoSettingsBeforeStartingAGame();
                } else {
                    LOGE("MENU: DoSettingsBeforeStartingAGame is NULL!");
                }
            }
        }
    }
}


// ---------------------------------------------------------------------
//  Подмена скрипта: при мультиплеере грузим main_mp.scm вместо main.scm.
//  Оригинал делает это в CTheScripts::Init через CFileMgr::OpenFile.
// ---------------------------------------------------------------------
typedef uint32_t (*OpenFile_t)(const char* name, const char* mode);
static OpenFile_t OpenFileOrig = nullptr;

static uint32_t OpenFileHook(const char* name, const char* mode)
{
    if (!OpenFileOrig) return 0;

    // Логируем только .scm — их немного, остальные файлы не засоряют лог
    if (name && strstr(name, ".scm"))
        LOGI("OpenFile: '%s' (mode=%s, mp=%d)", name, mode ? mode : "?",
             (int)CMultiplayer::bWantMP);

    // Подмена скрипта теперь идёт через ReadHook (абсолютный путь /sdcard).

    return OpenFileOrig(name, mode);
}


// ---------------------------------------------------------------------
//  Защита от NULL-handle: main_mp.scm открывает файлы, которых может не
//  быть (в re3 они есть, в GTA3-пакете нет). OpenFile вернёт 0, а Seek
//  без проверки падает по адресу 0x...8708.
// ---------------------------------------------------------------------
typedef uint64_t (*Seek_t)(uint64_t handle, int offset, int origin);
static Seek_t SeekOrig = nullptr;

static uint64_t SeekHook(uint64_t handle, int offset, int origin)
{
    if (handle == 0) {
        LOGI("CFileMgr::Seek: NULL handle, skipped (offset=%d origin=%d)", offset, origin);
        return 0;
    }
    return SeekOrig ? SeekOrig(handle, offset, origin) : 0;
}

typedef int (*Read_t)(uint64_t handle, void* buf, int size);
static Read_t ReadOrig = nullptr;

// Содержимое main_mp.scm, прочитанное обычным fopen (виртуальная ФС игры
// абсолютный /sdcard не понимает, а fopen — понимает).
static uint8_t* g_mpScriptData = nullptr;
static int      g_mpScriptSize = 0;
static int      g_mpScriptPos  = 0;

static bool LoadMpScriptToMemory()
{
    if (g_mpScriptData) return true;

    static const char* kPaths[] = {
        "/sdcard/Android/data/com.rockstar.gta3/files/GTA3/main_mp.scm",
        "/storage/emulated/0/Android/data/com.rockstar.gta3/files/GTA3/main_mp.scm",
    };
    for (size_t i = 0; i < sizeof(kPaths)/sizeof(kPaths[0]); i++) {
        FILE* f = fopen(kPaths[i], "rb");
        if (!f) continue;

        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (sz > 0 && sz <= 512 * 1024) {
            uint8_t* data = (uint8_t*)malloc((size_t)sz);
            if (data && fread(data, 1, (size_t)sz, f) == (size_t)sz) {
                g_mpScriptData = data;
                g_mpScriptSize = (int)sz;
                g_mpScriptPos  = 0;
                fclose(f);
                LOGI("Script: main_mp.scm loaded to memory (%d bytes) from %s",
                     g_mpScriptSize, kPaths[i]);
                return true;
            }
            free(data);
        }
        fclose(f);
    }
    LOGE("Script: cannot read main_mp.scm");
    return false;
}

static int ReadHook(uint64_t handle, void* buf, int size)
{
    // ВАЖНО: подменяем ТОЛЬКО первое чтение скрипта.
    // Read зовётся для всех файлов игры (модели, текстуры) — если
    // отдавать наш скрипт всем подряд, мир останется без текстур.
    // Подмена скрипта отключена: main_mp.scm собран под re3, в GTA3
    // камера падает (CCamera::UpdateTargetEntity -> FindPlayerPed == NULL).
    // Оригинальный main.scm создаёт игрока и грузит мир сам.
    (void)size;

    if (handle == 0) return 0;
    return ReadOrig ? ReadOrig(handle, buf, size) : 0;
}

void DoSettingsBeforeStartingAGameHook()
{
    if (DoSettingsBeforeStartingAGameOrig) {
        DoSettingsBeforeStartingAGameOrig();
    }
}

// ---------------------------------------------------------------------
//  Игровые обёртки
// ---------------------------------------------------------------------
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

// ---------------------------------------------------------------------
//  Установка хуков
// ---------------------------------------------------------------------
void InstallCallHooks()
{
    LOGI("Installing Dobby Hooks for Android Multiplayer...");

    void* pGetIcon = GetGameAddress(OFFSET_GetIconForOption);
    if (pGetIcon) {
        int r = DobbyHook(pGetIcon, (void*)GetIconForOptionHook, (void**)&pfnGetIconForOptionOrig);
        LOGI("GetIconForOption hook -> %d (%p)", r, pGetIcon);
    }

    void* pCTextGet = GetGameAddress(OFFSET_CText_Get);
    if (pCTextGet) {
        int r = DobbyHook(pCTextGet, (void*)CTextGetHook, (void**)&CTextGetOrig);
        LOGI("CText::Get hook -> %d (%p)", r, pCTextGet);
    }

    void* pRender2D = GetGameAddress(OFFSET_Render2dStuff);
    if (pRender2D) {
        int r = DobbyHook(pRender2D, (void*)Render2dStuffHook, (void**)&Render2dStuffOrig);
        LOGI("Render2dStuff hook -> %d (%p)", r, pRender2D);
    }

    void* eglSym = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
    if (eglSym) {
        int r = DobbyHook(eglSym, (void*)eglSwapBuffersHook, (void**)&eglSwapBuffersOrig);
        LOGI("eglSwapBuffers hook -> %d (%p)", r, eglSym);
    } else {
        LOGE("eglSwapBuffers NOT found!");
    }

    // CFont::PrintString — чтобы узнать координаты пунктов меню
    void* pPrint = GetGameAddress(OFFSET_CFont_PrintString);
    if (pPrint) {
        int r = DobbyHook(pPrint, (void*)PrintStringHook, (void**)&pfnPrintStringOrig);
        LOGI("CFont::PrintString hook -> %d (%p)", r, pPrint);
    }

    // Обработка выбора пункта меню
    void* pProc = GetGameAddress(OFFSET_ProcessButtonPresses);
    if (pProc) {
        int r = DobbyHook(pProc, (void*)ProcessButtonPressesHook, (void**)&ProcessButtonPressesOrig);
        LOGI("ProcessButtonPresses hook -> %d (%p)", r, pProc);
    }

    // Запуск игры из меню — без этого выбор MULTIPLAYER не грузит игру
    void* pStart = GetGameAddress(OFFSET_DoSettingsBeforeStartingAGame);
    if (pStart) {
        int r = DobbyHook(pStart, (void*)DoSettingsBeforeStartingAGameHook, (void**)&DoSettingsBeforeStartingAGameOrig);
        LOGI("DoSettingsBeforeStartingAGame hook -> %d (%p)", r, pStart);
        DoSettingsBeforeStartingAGame = (void(*)())pStart;
    } else {
        LOGE("DoSettingsBeforeStartingAGame NOT found!");
    }

    // Подмена main.scm -> main_mp.scm в мультиплеере
    void* pOpen = GetGameAddress(OFFSET_CFileMgr_OpenFile);
    if (pOpen) {
        int r = DobbyHook(pOpen, (void*)OpenFileHook, (void**)&OpenFileOrig);
        LOGI("CFileMgr::OpenFile hook -> %d (%p)", r, pOpen);
    } else {
        LOGE("CFileMgr::OpenFile NOT found!");
    }

    // Защита от NULL-handle в скриптовых файловых опкодах
    void* pSeek = GetGameAddress(OFFSET_CFileMgr_Seek);
    if (pSeek) {
        int r = DobbyHook(pSeek, (void*)SeekHook, (void**)&SeekOrig);
        LOGI("CFileMgr::Seek hook -> %d (%p)", r, pSeek);
    }
    void* pRead = GetGameAddress(OFFSET_CFileMgr_Read);
    if (pRead) {
        int r = DobbyHook(pRead, (void*)ReadHook, (void**)&ReadOrig);
        LOGI("CFileMgr::Read hook -> %d (%p)", r, pRead);
    }

    LOGI("All hooks installed.");
}
