#ifndef CPLAYERMANAGER_H
#define CPLAYERMANAGER_H

#include "CRemotePlayer.h"
#include <map>
#include <cstdint>

class CPlayerManager {
public:
    CPlayerManager();
    ~CPlayerManager();

    void AddPlayer(uint8_t id, const char* name);
    void RemovePlayer(uint8_t id);
    CRemotePlayer* GetPlayer(uint8_t id);

    void Process();
    void DrawNametags();

private:
    std::map<uint8_t, CRemotePlayer*> m_Players;
};

extern CPlayerManager* pPlayerManager;

#endif
