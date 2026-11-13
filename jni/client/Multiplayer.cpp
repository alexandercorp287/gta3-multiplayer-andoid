#include "Multiplayer.h"
#include "CNetClient.h"
#include "CLocalPlayer.h"
#include "CPlayerManager.h"
#include "../gui/CChatWindow.h"
#include "../gui/CDialogWindow.h"
#include "../gui/CSpawnScreen.h"
#include "../main.h"
#include "keyboard.h"
#include "../gui/gui.h"

bool CMultiplayer::bWantMP = false;
bool CMultiplayer::bIsActive = false;
bool CMultiplayer::bPendingSpawn = false;

char CMultiplayer::szNickName[32] = "AndroidPlayer";
char CMultiplayer::szServerIP[128] = "178.250.156.53";
int  CMultiplayer::iServerPort = 5555;

// ---------------------------------------------------------------------
//  Настройки: /sdcard/3mp_config.txt
//    nick=Player
//    ip=1.2.3.4
//    port=5555
// ---------------------------------------------------------------------
static const char* kConfigPaths[] = {
    // Родная папка приложения — доступна на запись без MANAGE_EXTERNAL_STORAGE
    "/sdcard/Android/data/com.rockstar.gta3/files/GTA3/3mp_config.txt",
    "/storage/emulated/0/Android/data/com.rockstar.gta3/files/GTA3/3mp_config.txt",
    // Резерв: корень SD (может быть недоступен на Android 11+)
    "/sdcard/3mp_config.txt",
    "/data/local/tmp/3mp_config.txt",
};

void CMultiplayer::LoadMPSettings()
{
    FILE* f = nullptr;
    const char* used = nullptr;

    for (size_t i = 0; i < sizeof(kConfigPaths) / sizeof(kConfigPaths[0]); i++) {
        LOGI("Config try: %s", kConfigPaths[i]);
        f = fopen(kConfigPaths[i], "r");
        if (f) { used = kConfigPaths[i]; break; }
    }

    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char key[64], val[128];
            if (sscanf(line, "%63[^=]=%127s", key, val) == 2) {
                if (strcmp(key, "nick") == 0) {
                    strncpy(CMultiplayer::szNickName, val, 31);
                    CMultiplayer::szNickName[31] = 0;
                } else if (strcmp(key, "ip") == 0) {
                    strncpy(CMultiplayer::szServerIP, val, 127);
                    CMultiplayer::szServerIP[127] = 0;
                } else if (strcmp(key, "port") == 0) {
                    CMultiplayer::iServerPort = atoi(val);
                }
            }
        }
        fclose(f);
        LOGI("Config LOADED from %s: nick=%s ip=%s port=%d",
             used, CMultiplayer::szNickName, CMultiplayer::szServerIP,
             CMultiplayer::iServerPort);
        return;
    }

    // Файла нет — не пытаемся создавать (Android 11+ блокирует запись).
    // Работаем с встроенными настройками, путь для ручного создания сообщаем в лог.
    LOGI("Config not found. Using defaults: nick=%s ip=%s port=%d",
         CMultiplayer::szNickName, CMultiplayer::szServerIP,
         CMultiplayer::iServerPort);
    LOGI("Put 3mp_config.txt here to override: %s", kConfigPaths[0]);
}

void CMultiplayer::SaveMPSettings()
{
    // Пишем в первый доступный путь (папка приложения доступна без разрешений)
    for (size_t i = 0; i < sizeof(kConfigPaths) / sizeof(kConfigPaths[0]); i++) {
        FILE* f = fopen(kConfigPaths[i], "w");
        if (!f) continue;
        fprintf(f, "nick=%s\nip=%s\nport=%d\n",
                szNickName, szServerIP, iServerPort);
        fclose(f);
        LOGI("Config saved: %s @ %s:%d -> %s", szNickName, szServerIP, iServerPort,
             kConfigPaths[i]);
        return;
    }
    LOGE("Config save failed: no writable path");
}

void CMultiplayer::Initialise()
{
    if (bIsActive) return;

    LOGI("CMultiplayer: Initialise() called");

    CMultiplayer::LoadMPSettings();

    LOGI("Multiplayer: GUI init...");
    if (!pGUI) pGUI = new CGui();
    if (pGUI && !pGUI->m_bInited) pGUI->Init();
    LOGI("Multiplayer: GUI ready (inited=%d)", pGUI ? (int)pGUI->m_bInited : -1);

    if (!pChatWindow)   pChatWindow   = new CChatWindow();
    if (!pSpawnScreen)  pSpawnScreen  = new CSpawnScreen();
    if (!pDialogWindow) pDialogWindow = new CDialogWindow();

    if (!pKeyBoard)      pKeyBoard      = new CKeyBoard();

    if (!pNetClient)     pNetClient     = new CNetClient();
    if (!pLocalPlayer)   pLocalPlayer   = new CLocalPlayer();
    if (!pPlayerManager) pPlayerManager = new CPlayerManager();

    if (pChatWindow) pChatWindow->AddInfoMessage("Multi-Player Initialized.");
    if (pChatWindow) pChatWindow->AddInfoMessage("Connecting to %s:%d ...",
                                                 szServerIP, iServerPort);

    if (pNetClient) {
        pNetClient->Connect(szServerIP, iServerPort);
        if (pChatWindow) {
            if (pNetClient->IsConnected())
                pChatWindow->AddInfoMessage("Connected. Waiting for server...");
            else
                pChatWindow->AddInfoMessage("Network Error: connect failed.");
        }
    }

    bIsActive = true;
    LOGI("CMultiplayer: System started (active=%d)", (int)bIsActive);
}

void CMultiplayer::Process()
{
    if (!bIsActive) return;

    if (bPendingSpawn) {
        bPendingSpawn = false;
        LOGI("CMultiplayer: pending spawn -> CLocalPlayer::Spawn()");
        if (pLocalPlayer) pLocalPlayer->Spawn();
    }

    if (pNetClient)     pNetClient->Process();
    if (pLocalPlayer)   pLocalPlayer->Process();
    if (pPlayerManager) pPlayerManager->Process();
}

void CMultiplayer::Shutdown()
{
    if (!bIsActive) return;

    LOGI("CMultiplayer: Shutdown()");

    if (pDialogWindow)  { delete pDialogWindow;  pDialogWindow  = nullptr; }
    if (pSpawnScreen)   { delete pSpawnScreen;   pSpawnScreen   = nullptr; }
    if (pPlayerManager) { delete pPlayerManager; pPlayerManager = nullptr; }
    if (pLocalPlayer)   { delete pLocalPlayer;   pLocalPlayer   = nullptr; }
    if (pNetClient)     { delete pNetClient;     pNetClient     = nullptr; }
    if (pKeyBoard)      { delete pKeyBoard;      pKeyBoard      = nullptr; }
    if (pChatWindow)    { delete pChatWindow;    pChatWindow    = nullptr; }

    bIsActive = false;
}
