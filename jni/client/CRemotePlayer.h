#ifndef CREMOTEPLAYER_H
#define CREMOTEPLAYER_H

#include "NetProtocol.h"
#include <string>

class CRemotePlayer {
public:
    CRemotePlayer(uint8_t id, const char* name);
    ~CRemotePlayer();

    void Spawn(int model, float x, float y, float z, float rot);
    void UpdateSync(OnFootSyncData* data);
    void Process();

    uint8_t GetPlayerId() const { return m_nPlayerId; }
    const char* GetName() const { return m_szName; }

private:
    uint8_t m_nPlayerId;
    char m_szName[32];
    Vector3 m_vecPos;
    float m_fRotation;
    void* m_pPed;
};

#endif
