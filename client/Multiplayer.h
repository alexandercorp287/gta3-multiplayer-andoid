#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include <cstdint>

class CMultiplayer {
public:
    static void Initialise();
    static void Process();
    static void Shutdown();

    static bool IsActive() { return bIsActive; }

    static bool bWantMP;
    static bool bIsActive;
    static bool bPendingSpawn;
    static char szNickName[32];
    static char szServerIP[128];
    static int iServerPort;
};

#endif
