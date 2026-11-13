#include "CNetClient.h"
#include "../main.h"
#include "../gui/CChatWindow.h"
#include "../gui/CDialogWindow.h"
#include "../gui/CSpawnScreen.h"
#include "RakNetworkFactory.h"
#include "PacketEnumerations.h"
#include "CLocalPlayer.h"
#include "CPlayerManager.h"
#include "CRemotePlayer.h"
#include "Multiplayer.h"

CNetClient* pNetClient = nullptr;

CNetClient::CNetClient() {
    m_pRakClient = RakNetworkFactory::GetRakClientInterface();
    m_bConnected = false;
}

CNetClient::~CNetClient() {
    Disconnect();
    if (m_pRakClient) {
        RakNetworkFactory::DestroyRakClientInterface(m_pRakClient);
        m_pRakClient = nullptr;
    }
}

void CNetClient::Connect(const char* szHost, int iPort, const char* szPass) {
    if (!m_pRakClient) return;

    m_bConnected = false;
    LOGI("CNetClient: Connecting to %s:%d...", szHost, iPort);
    if (pChatWindow) pChatWindow->AddInfoMessage("Connecting to %s:%d...", szHost, iPort);

    bool res = m_pRakClient->Connect(szHost, (unsigned short)iPort, 0, 0, 10);
    if (!res) {
        LOGE("CNetClient: RakClient Connect FAILED!");
        if (pChatWindow) pChatWindow->AddInfoMessage("Network Error: Connection failed.");
    }
}

void CNetClient::Disconnect() {
    if (m_pRakClient && m_bConnected) {
        m_pRakClient->Disconnect(100);
        m_bConnected = false;
        if (pChatWindow) pChatWindow->AddInfoMessage("Disconnected from server.");
    }
}

void CNetClient::SendPacket(RakNet::BitStream* bs, PacketPriority priority, PacketReliability reliability) {
    if (m_pRakClient && m_bConnected && bs) {
        m_pRakClient->Send(bs, priority, reliability, 0);
    }
}

void CNetClient::Process() {
    if (!m_pRakClient) return;

    Packet* p = nullptr;
    while ((p = m_pRakClient->Receive()) != nullptr) {
        HandleRakNetPacket(p);
        m_pRakClient->DeallocatePacket(p);
    }
}

void CNetClient::HandleRakNetPacket(Packet* p) {
    if (!p) return;
    unsigned char type = p->data[0];

    switch (type) {
        case ID_CONNECTION_REQUEST_ACCEPTED: {
            m_bConnected = true;
            LOGI("CNetClient: Connection accepted!");
            if (pChatWindow) pChatWindow->AddInfoMessage("Connected to server!");

            RakNet::BitStream bs;
            bs.Write((unsigned char)ID_PLAYER_LOGIN_VMP);
            uint8_t nLen = (uint8_t)strlen(CMultiplayer::szNickName);
            bs.Write(nLen);
            bs.Write(CMultiplayer::szNickName, (int)nLen);
            SendPacket(&bs);
            break;
        }

        case ID_DISCONNECTION_NOTIFICATION:
            LOGI("CNetClient: Disconnected.");
            Disconnect();
            break;

        case ID_CONNECTION_LOST:
            LOGI("CNetClient: Connection lost.");
            Disconnect();
            break;

        case ID_CONNECTION_SET_ID_VMP: {
            if (p->length < 2) break;
            uint8_t myId = p->data[1];
            if (pLocalPlayer) pLocalPlayer->SetPlayerId(myId);
            LOGI("CNetClient: My local player ID: %d", (int)myId);
            break;
        }

        case ID_NEW_PLAYER_VMP: {
            if (p->length < 34) break;
            uint8_t playerId = p->data[1];
            if (pLocalPlayer && playerId == pLocalPlayer->GetPlayerId()) break;

            char nick[33] = {0};
            memcpy(nick, p->data + 2, 32);
            nick[32] = '\0';

            if (pPlayerManager) pPlayerManager->AddPlayer(playerId, nick);
            LOGI("CNetClient: New player %s (ID:%d) connected.", nick, playerId);
            break;
        }

        case ID_PLAYER_QUIT_VMP: {
            if (p->length < 2) break;
            uint8_t playerId = p->data[1];
            if (pPlayerManager) pPlayerManager->RemovePlayer(playerId);
            LOGI("CNetClient: Player ID %d disconnected.", playerId);
            break;
        }

        case ID_PLAYER_SPAWN_VMP: {
            if (p->length >= 2) {
                uint8_t playerId = p->data[1];
                if (pLocalPlayer && playerId == pLocalPlayer->GetPlayerId()) {
                    CMultiplayer::bPendingSpawn = true;
                    if (pSpawnScreen) pSpawnScreen->Show(false);
                } else if (pPlayerManager) {
                    CRemotePlayer* remote = pPlayerManager->GetPlayer(playerId);
                    if (!remote) {
                        char szDefaultName[32];
                        snprintf(szDefaultName, 32, "Player_%d", (int)playerId);
                        pPlayerManager->AddPlayer(playerId, szDefaultName);
                        remote = pPlayerManager->GetPlayer(playerId);
                    }
                    if (remote) {
                        int model = 7;
                        float x = 0.0f, y = 0.0f, z = 0.0f, rot = 0.0f;
                        if (p->length >= 2 + sizeof(int) + sizeof(float) * 4) {
                            RakNet::BitStream bs(p->data, p->length, false);
                            bs.IgnoreBits(16);
                            bs.Read(model); bs.Read(x); bs.Read(y); bs.Read(z); bs.Read(rot);
                        }
                        remote->Spawn(model, x, y, z, rot);
                        LOGI("CNetClient: Remote player %d spawned at (%.1f, %.1f, %.1f)", (int)playerId, x, y, z);
                    }
                }
            }
            break;
        }

        case ID_PLAYER_SYNC_VMP: {
            if (p->length < 2 + sizeof(OnFootSyncData)) break;
            uint8_t playerId = p->data[1];
            if (pLocalPlayer && playerId == pLocalPlayer->GetPlayerId()) break;

            if (pPlayerManager) {
                CRemotePlayer* remote = pPlayerManager->GetPlayer(playerId);
                if (!remote) {
                    char szDefaultName[32];
                    snprintf(szDefaultName, 32, "Player_%d", (int)playerId);
                    pPlayerManager->AddPlayer(playerId, szDefaultName);
                    remote = pPlayerManager->GetPlayer(playerId);
                }
                if (remote) {
                    OnFootSyncData sd;
                    memcpy(&sd, p->data + 2, sizeof(OnFootSyncData));
                    remote->UpdateSync(&sd);
                }
            }
            break;
        }

        case ID_CHAT_MESSAGE_VMP: {
            if (p->length < 6) break;
            uint32_t color = 0;
            memcpy(&color, p->data + 1, 4);
            uint8_t nickLen = p->data[5];
            char nick[128] = {0};
            if (nickLen > 0 && nickLen < 127 && p->length >= 6 + nickLen + 1) {
                memcpy(nick, p->data + 6, nickLen);
                uint8_t msgLen = p->data[6 + nickLen];
                char msg[512] = {0};
                if (msgLen > 0 && msgLen < 511) {
                    memcpy(msg, p->data + 6 + nickLen + 1, msgLen);
                    if (pChatWindow) pChatWindow->AddChatMessage(nick, color, msg);
                }
            }
            break;
        }

        case ID_SHOW_DIALOG_VMP: {
            RakNet::BitStream bs(p->data, p->length, false);
            bs.IgnoreBits(8);
            uint16_t dialogId; uint8_t style;
            bs.Read(dialogId); bs.Read(style);
            uint8_t tLen = 0, iLen = 0, b1Len = 0, b2Len = 0;
            char title[128] = {0}, info[4096] = {0}, b1[64] = {0}, b2[64] = {0};
            bs.Read(tLen); if (tLen > 0 && tLen < 127) { bs.Read(title, (int)tLen); title[tLen] = '\0'; }
            bs.Read(iLen); if (iLen > 0 && iLen < 4095) { bs.Read(info, (int)iLen); info[iLen] = '\0'; }
            bs.Read(b1Len); if (b1Len > 0 && b1Len < 63) { bs.Read(b1, (int)b1Len); b1[b1Len] = '\0'; }
            bs.Read(b2Len); if (b2Len > 0 && b2Len < 63) { bs.Read(b2, (int)b2Len); b2[b2Len] = '\0'; }
            if (pDialogWindow) {
                pDialogWindow->ShowDialog(dialogId, style, title, info, b1, b2);
            }
            break;
        }

        case ID_FORCE_CLASS_SELECTION: {
            if (pSpawnScreen) pSpawnScreen->Show(true);
            CMultiplayer::bPendingSpawn = false;
            break;
        }
    }
}
