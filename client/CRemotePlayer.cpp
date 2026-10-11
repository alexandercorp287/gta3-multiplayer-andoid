#include "CRemotePlayer.h"
#include "../game/game.h"
#include "../main.h"

CRemotePlayer::CRemotePlayer(uint8_t id, const char* name) {
    m_nPlayerId = id;
    m_pPed = nullptr;
    m_fRotation = 0.0f;
    m_vecPos = {0.0f, 0.0f, 0.0f};
    memset(m_szName, 0, sizeof(m_szName));
    if (name) strncpy(m_szName, name, 31);
}

CRemotePlayer::~CRemotePlayer() {
}

void CRemotePlayer::Spawn(int model, float x, float y, float z, float rot) {
    m_vecPos = {x, y, z};
    m_fRotation = rot;
    LOGI("CRemotePlayer::Spawn id=%d name=%s at (%.1f, %.1f, %.1f)", (int)m_nPlayerId, m_szName, x, y, z);
}

void CRemotePlayer::UpdateSync(OnFootSyncData* data) {
    if (!data) return;
    m_vecPos = data->pos;
    m_fRotation = data->rotation;
}

void CRemotePlayer::Process() {
}
