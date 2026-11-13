#include "CSpawnScreen.h"
#include "../client/CLocalPlayer.h"
#include "../client/CNetClient.h"
#include "../vendor/imgui/imgui.h"

CSpawnScreen* pSpawnScreen = nullptr;

CSpawnScreen::CSpawnScreen() {
    m_bEnabled = true;
}

CSpawnScreen::~CSpawnScreen() {
}

void CSpawnScreen::Draw() {
    if (!m_bEnabled) return;

    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;

    if (screenW <= 0 || screenH <= 0) return;

    ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f, screenH * 0.9f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(screenW * 0.4f, 80.0f));

    if (ImGui::Begin("SpawnScreen", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground)) {
        float btnW = (screenW * 0.4f) / 3.0f - 10.0f;

        if (ImGui::Button("<<", ImVec2(btnW, 60))) {
        }
        ImGui::SameLine();
        if (ImGui::Button("SPAWN", ImVec2(btnW, 60))) {
            if (pNetClient && pNetClient->IsConnected()) {
                RakNet::BitStream bs;
                bs.Write((unsigned char)ID_PLAYER_REQUEST_SPAWN);
                pNetClient->SendPacket(&bs);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(">>", ImVec2(btnW, 60))) {
        }
    }
    ImGui::End();
}
