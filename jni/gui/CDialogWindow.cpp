#include "CDialogWindow.h"
#include "../client/CNetClient.h"
#include "../vendor/imgui/imgui.h"
#include <cstdio>
#include <cstring>

CDialogWindow* pDialogWindow = nullptr;

CDialogWindow::CDialogWindow() {
    Clear();
}

CDialogWindow::~CDialogWindow() {
    Clear();
}

void CDialogWindow::Clear() {
    m_bIsActive = false;
    m_wDialogID = 0;
    m_byteDialogStyle = 0;
    m_iSelectedListItem = 0;
    memset(m_szTitle, 0, sizeof(m_szTitle));
    memset(m_szInfo, 0, sizeof(m_szInfo));
    memset(m_szButton1, 0, sizeof(m_szButton1));
    memset(m_szButton2, 0, sizeof(m_szButton2));
    memset(m_szInput, 0, sizeof(m_szInput));
}

void CDialogWindow::ShowDialog(uint16_t wDialogID, uint8_t byteStyle, const char* title, const char* info, const char* button1, const char* button2) {
    Clear();
    m_wDialogID = wDialogID;
    m_byteDialogStyle = byteStyle;

    if (title) strncpy(m_szTitle, title, sizeof(m_szTitle) - 1);
    if (info) strncpy(m_szInfo, info, sizeof(m_szInfo) - 1);
    if (button1) strncpy(m_szButton1, button1, sizeof(m_szButton1) - 1);
    if (button2) strncpy(m_szButton2, button2, sizeof(m_szButton2) - 1);

    Show(true);
}

void CDialogWindow::Render() {
    if (!m_bIsActive) return;

    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;

    if (screenW <= 0 || screenH <= 0) return;

    ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f, screenH * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.12f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.12f, 0.28f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.15f, 0.35f, 0.68f, 1.0f));

    char winTitle[128];
    snprintf(winTitle, sizeof(winTitle), "%s##vmp_dlg_%d", m_szTitle[0] ? m_szTitle : "Dialog", (int)m_wDialogID);

    if (ImGui::Begin(winTitle, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        if (m_szInfo[0] != '\0') {
            ImGui::TextWrapped("%s", m_szInfo);
            ImGui::Spacing();
        }

        if (m_byteDialogStyle == DIALOG_STYLE_INPUT || m_byteDialogStyle == DIALOG_STYLE_PASSWORD) {
            ImGui::InputText("##dialoginput", m_szInput, sizeof(m_szInput),
                m_byteDialogStyle == DIALOG_STYLE_PASSWORD ? ImGuiInputTextFlags_Password : 0);
            ImGui::Spacing();
        }

        ImGui::Separator();
        ImGui::Spacing();

        if (m_szButton1[0] != '\0') {
            if (ImGui::Button(m_szButton1, ImVec2(120, 42))) {
                Show(false);
                if (pNetClient && pNetClient->IsConnected()) {
                    RakNet::BitStream bs;
                    bs.Write((unsigned char)ID_DIALOG_RESPONSE_VMP);
                    bs.Write((uint16_t)m_wDialogID);
                    bs.Write((uint8_t)1);
                    bs.Write((uint16_t)m_iSelectedListItem);
                    uint8_t len = (uint8_t)strlen(m_szInput);
                    bs.Write(len);
                    if (len > 0) bs.Write(m_szInput, (int)len);
                    pNetClient->SendPacket(&bs);
                }
                Clear();
            }
        }

        if (m_szButton2[0] != '\0') {
            ImGui::SameLine(0, 15.0f);
            if (ImGui::Button(m_szButton2, ImVec2(120, 42))) {
                Show(false);
                if (pNetClient && pNetClient->IsConnected()) {
                    RakNet::BitStream bs;
                    bs.Write((unsigned char)ID_DIALOG_RESPONSE_VMP);
                    bs.Write((uint16_t)m_wDialogID);
                    bs.Write((uint8_t)0);
                    bs.Write((uint16_t)m_iSelectedListItem);
                    uint8_t len = (uint8_t)strlen(m_szInput);
                    bs.Write(len);
                    if (len > 0) bs.Write(m_szInput, (int)len);
                    pNetClient->SendPacket(&bs);
                }
                Clear();
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
