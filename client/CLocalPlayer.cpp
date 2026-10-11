#include "CLocalPlayer.h"
#include "CNetClient.h"
#include "../game/game.h"
#include "../main.h"

CLocalPlayer* pLocalPlayer = nullptr;

CLocalPlayer::CLocalPlayer() {
    m_nPlayerId = 255;
    m_dwLastSendTick = 0;
}

CLocalPlayer::~CLocalPlayer() {
}

void CLocalPlayer::Spawn() {
    LOGI("CLocalPlayer::Spawn called.");
}

void CLocalPlayer::SendOnFootFullSyncData() {
    if (m_nPlayerId == 255) return;

    void* pPed = GetLocalPlayerPed();
    if (!pPed) return;

    OnFootSyncData sync;
    memset(&sync, 0, sizeof(OnFootSyncData));

    float* pPos = (float*)((uintptr_t)pPed + 0x34);
    sync.pos.x = pPos[0];
    sync.pos.y = pPos[1];
    sync.pos.z = pPos[2];

    sync.health = 100;
    sync.armour = 0;
    sync.weapon = 0;

    if (pNetClient && pNetClient->IsConnected()) {
        RakNet::BitStream bs;
        bs.Write((unsigned char)ID_PLAYER_SYNC_VMP);
        bs.Write((const char*)&sync, sizeof(OnFootSyncData));
        pNetClient->SendPacket(&bs, HIGH_PRIORITY, UNRELIABLE);
    }
}

void CLocalPlayer::Process() {
    if (!pNetClient || !pNetClient->IsConnected()) return;

    SendOnFootFullSyncData();
}
