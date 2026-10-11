#include "Multiplayer.h"
#include "CNetClient.h"
#include "CLocalPlayer.h"
#include "CPlayerManager.h"
#include "../gui/CChatWindow.h"
#include "../gui/CDialogWindow.h"
#include "../gui/CSpawnScreen.h"
#include "../main.h"

bool CMultiplayer::bWantMP = false;
bool CMultiplayer::bIsActive = false;
bool CMultiplayer::bPendingSpawn = false;

char CMultiplayer::szNickName[32] = "AndroidPlayer";
char CMultiplayer::szServerIP[128] = "178.250.156.53";
int CMultiplayer::iServerPort = 5555;

void CMultiplayer::Initialise() {
    if (bIsActive) return;

    LOGI("CMultiplayer: Initializing components...");

    pChatWindow = new CChatWindow();
    pSpawnScreen = new CSpawnScreen();
    pDialogWindow = new CDialogWindow();

    pNetClient = new CNetClient();
    pLocalPlayer = new CLocalPlayer();
    pPlayerManager = new CPlayerManager();

    if (pNetClient) {
        pNetClient->Connect(szServerIP, iServerPort);
    }

    bIsActive = true;
    LOGI("CMultiplayer: System started successfully!");
}

void CMultiplayer::Process() {
    if (!bIsActive) return;

    if (pNetClient) pNetClient->Process();
    if (pLocalPlayer) pLocalPlayer->Process();
    if (pPlayerManager) pPlayerManager->Process();
}

void CMultiplayer::Shutdown() {
    if (!bIsActive) return;

    if (pDialogWindow) { delete pDialogWindow; pDialogWindow = nullptr; }
    if (pSpawnScreen) { delete pSpawnScreen; pSpawnScreen = nullptr; }
    if (pPlayerManager) { delete pPlayerManager; pPlayerManager = nullptr; }
    if (pLocalPlayer) { delete pLocalPlayer; pLocalPlayer = nullptr; }
    if (pNetClient) { delete pNetClient; pNetClient = nullptr; }
    if (pChatWindow) { delete pChatWindow; pChatWindow = nullptr; }

    bIsActive = false;
}
