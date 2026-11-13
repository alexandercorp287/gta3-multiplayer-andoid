#include "ServerList.h"
#include "../main.h"
#include "../client/Multiplayer.h"
#include "../vendor/imgui/imgui.h"

static bool  s_bOpen = false;
static char  s_szIp[128]   = "";
static char  s_szPort[16]  = "5555";
static char  s_szNick[32]  = "";
static int   s_iSelected   = -1;

// Список серверов: читается из того же конфига, что и настройки.
// Формат строки в списке: ip:port  [название]
struct ServerEntry { const char* ip; int port; const char* name; };
static const ServerEntry kServers[] = {
    { "127.0.0.1",       5555, "Local server"      },
    { "178.250.156.53",  5555, "Old re3 (offline)" },
};

bool ServerList_IsOpen() { return s_bOpen; }
void ServerList_Close()  { s_bOpen = false; }

// Открыть окно (вызывается из хука меню)
void ServerList_Open()
{
    s_szIp[0] = 0;          // при открытии подтянуть актуальные настройки
    s_bOpen = true;
    LOGI("ServerList: opened");
}

static void StartGame()
{
    if (DoSettingsBeforeStartingAGame) {
        DoSettingsBeforeStartingAGame();
    } else {
        LOGE("ServerList: DoSettingsBeforeStartingAGame is NULL!");
    }
}

void ServerList_Render()
{
    if (!s_bOpen) return;

    // При первом открытии подставляем текущие настройки
    if (s_szIp[0] == 0) {
        strncpy(s_szIp,  CMultiplayer::szServerIP, sizeof(s_szIp) - 1);
        strncpy(s_szNick, CMultiplayer::szNickName, sizeof(s_szNick) - 1);
        snprintf(s_szPort, sizeof(s_szPort), "%d", CMultiplayer::iServerPort);
    }

    ImGuiIO& io = ImGui::GetIO();
    float w = io.DisplaySize.x, h = io.DisplaySize.y;
    if (w <= 0 || h <= 0) { w = 1570.0f; h = 720.0f; }

    ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(w * 0.70f, h * 0.80f), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.05f, 0.04f, 0.97f));
    ImGui::PushStyleColor(ImGuiCol_Border,   ImVec4(0.82f, 0.68f, 0.38f, 1.00f));

    if (ImGui::Begin("MULTIPLAYER", &s_bOpen, flags)) {

        // ---- Ник ----
        ImGui::Text("Nickname");
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText("##nick", s_szNick, sizeof(s_szNick));

        ImGui::Spacing();

        // ---- Список серверов ----
        ImGui::Text("Servers");
        if (ImGui::BeginChild("##list", ImVec2(0, h * 0.28f), true)) {
            for (int i = 0; i < (int)(sizeof(kServers)/sizeof(kServers[0])); i++) {
                char label[160];
                snprintf(label, sizeof(label), "%-16s :%-5d  %s",
                         kServers[i].ip, kServers[i].port, kServers[i].name);
                if (ImGui::Selectable(label, s_iSelected == i)) {
                    s_iSelected = i;
                    strncpy(s_szIp, kServers[i].ip, sizeof(s_szIp) - 1);
                    snprintf(s_szPort, sizeof(s_szPort), "%d", kServers[i].port);
                }
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();

        // ---- Адрес вручную ----
        ImGui::Text("Address");
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText("##ip", s_szIp, sizeof(s_szIp));
        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::InputText("##port", s_szPort, sizeof(s_szPort));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ---- Кнопки ----
        if (ImGui::Button("CONNECT", ImVec2(220, 55))) {
            int port = atoi(s_szPort);
            if (port <= 0 || port > 65535) port = 5555;

            strncpy(CMultiplayer::szNickName, s_szNick, sizeof(CMultiplayer::szNickName) - 1);
            strncpy(CMultiplayer::szServerIP, s_szIp, sizeof(CMultiplayer::szServerIP) - 1);
            CMultiplayer::iServerPort = port;
            CMultiplayer::SaveMPSettings();

            LOGI("ServerList: CONNECT %s:%d as '%s'", s_szIp, port, s_szNick);

            s_bOpen = false;
            CMultiplayer::bWantMP = true;
            CMultiplayer::Initialise();

            // Запуск самой игры
            if (DoSettingsBeforeStartingAGame) DoSettingsBeforeStartingAGame();
        }

        ImGui::SameLine();
        if (ImGui::Button("CANCEL", ImVec2(180, 55))) {
            LOGI("ServerList: cancelled");
            s_bOpen = false;
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
}