#ifndef CLOCALPLAYER_H
#define CLOCALPLAYER_H

#include "NetProtocol.h"

class CLocalPlayer {
public:
    CLocalPlayer();
    ~CLocalPlayer();

    void Process();
    void SendOnFootFullSyncData();
    void Spawn();

    void SetPlayerId(uint8_t id) { m_nPlayerId = id; }
    uint8_t GetPlayerId() const { return m_nPlayerId; }

private:
    uint8_t m_nPlayerId;
    uint32_t m_dwLastSendTick;
};

extern CLocalPlayer* pLocalPlayer;

#endif
