#include "CChatWindow.h"
#include "../client/CNetClient.h"
#include "../vendor/imgui/imgui.h"
#include <cstdio>
#include <cstdarg>

CChatWindow* pChatWindow = nullptr;

CChatWindow::CChatWindow() {
}

CChatWindow::~CChatWindow() {
    m_Messages.clear();
}

void CChatWindow::AddChatMessage(const char* sender, uint32_t color, const char* text) {
    if (!text) return;
    ChatMessage msg;
    msg.sender = sender ? sender : "";
    msg.color = color;
    msg.text = text;
    m_Messages.push_back(msg);
    if (m_Messages.size() > 50) m_Messages.erase(m_Messages.begin());
}

void CChatWindow::AddInfoMessage(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    AddChatMessage("Server", 0x00FF00FF, buf);
}

void CChatWindow::Draw() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(450, 200), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("ChatWindow", nullptr, flags)) {
        for (const auto& msg : m_Messages) {
            if (!msg.sender.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "[%s]: %s", msg.sender.c_str(), msg.text.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "* %s", msg.text.c_str());
            }
        }
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::End();
}
