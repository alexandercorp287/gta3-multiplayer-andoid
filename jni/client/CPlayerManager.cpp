#include "CPlayerManager.h"
#include "../main.h"

CPlayerManager* pPlayerManager = nullptr;

CPlayerManager::CPlayerManager() {
}

CPlayerManager::~CPlayerManager() {
    for (auto it : m_Players) {
        if (it.second) delete it.second;
    }
    m_Players.clear();
}

void CPlayerManager::AddPlayer(uint8_t id, const char* name) {
    RemovePlayer(id);
    m_Players[id] = new CRemotePlayer(id, name);
    LOGI("CPlayerManager: Added player ID %d (%s)", (int)id, name ? name : "Player");
}

void CPlayerManager::RemovePlayer(uint8_t id) {
    auto it = m_Players.find(id);
    if (it != m_Players.end()) {
        if (it->second) delete it->second;
        m_Players.erase(it);
        LOGI("CPlayerManager: Removed player ID %d", (int)id);
    }
}

CRemotePlayer* CPlayerManager::GetPlayer(uint8_t id) {
    auto it = m_Players.find(id);
    if (it != m_Players.end()) return it->second;
    return nullptr;
}

void CPlayerManager::Process() {
    for (auto it : m_Players) {
        if (it.second) it.second->Process();
    }
}

void CPlayerManager::DrawNametags() {
}
