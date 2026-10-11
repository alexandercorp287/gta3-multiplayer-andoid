#include "gui.h"
#include "../client/Multiplayer.h"
#include "../client/CPlayerManager.h"
#include "CChatWindow.h"
#include "CDialogWindow.h"
#include "CSpawnScreen.h"
#include "../main.h"

CGui* pGUI = nullptr;

CGui::CGui() {
    m_bInited = false;
}

CGui::~CGui() {
    if (m_bInited) {
        ImGui_ImplOpenGL3_Shutdown();
        if (ImGui::GetCurrentContext()) {
            ImGui::DestroyContext();
        }
    }
}

void CGui::Init() {
    if (m_bInited) return;

    IMGUI_CHECKVERSION();
    if (ImGui::GetCurrentContext() == nullptr) {
        ImGui::CreateContext();
    }

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    ImGui::StyleColorsDark();

    if (ImGui_ImplOpenGL3_Init("#version 300 es")) {
        m_bInited = true;
        LOGI("CGui::Init: ImGui & OpenGL ES3 initialized successfully!");
    }
}

void CGui::RenderMenuMultiplayerButton() {
    if (!m_bInited) return;

    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;

    if (screenW <= 0 || screenH <= 0) {
        screenW = 1920.0f;
        screenH = 1080.0f;
    }

    if (!CMultiplayer::IsActive()) {
        // Отрисовка ImGui кнопки MULTIPLAYER прямо в Главном Меню!
        ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f, screenH * 0.38f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(220.0f, 65.0f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.08f, 0.06f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.82f, 0.68f, 0.38f, 1.0f)); // Золотой оклад GTA III 10th

        if (ImGui::Begin("StartMPButtonWin", nullptr, flags)) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.14f, 0.09f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.28f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.55f, 0.45f, 0.22f, 1.0f));

            if (ImGui::Button("MULTIPLAYER", ImVec2(-1, -1))) {
                LOGI("GUI: Main Menu MULTIPLAYER ImGui button tapped!");
                CMultiplayer::bWantMP = true;
                CMultiplayer::Initialise();
            }

            ImGui::PopStyleColor(3);
        }
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }
}

void CGui::Render() {
    if (!m_bInited) {
        Init();
        if (!m_bInited) return;
    }

    if (CMultiplayer::IsActive()) {
        if (pChatWindow) pChatWindow->Draw();
        if (pPlayerManager) pPlayerManager->DrawNametags();
        if (pSpawnScreen) pSpawnScreen->Draw();
        if (pDialogWindow) pDialogWindow->Render();
    }
}
