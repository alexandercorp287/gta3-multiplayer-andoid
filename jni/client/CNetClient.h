#ifndef CNETCLIENT_H
#define CNETCLIENT_H

#include "RakClientInterface.h"
#include "NetProtocol.h"
#include <string>

class CNetClient {
public:
    CNetClient();
    ~CNetClient();

    void Connect(const char* szHost, int iPort, const char* szPass = nullptr);
    void Disconnect();
    void Process();

    bool IsConnected() const { return m_bConnected; }
    void SendPacket(RakNet::BitStream* bs, PacketPriority priority = HIGH_PRIORITY, PacketReliability reliability = RELIABLE_ORDERED);

    RakClientInterface* GetRakClient() { return m_pRakClient; }

private:
    void HandleRakNetPacket(Packet* p);

    RakClientInterface* m_pRakClient;
    bool m_bConnected;
};

extern CNetClient* pNetClient;

#endif
