#ifndef CCHATWINDOW_H
#define CCHATWINDOW_H

#include <string>
#include <vector>
#include <cstdint>

struct ChatMessage {
    std::string sender;
    uint32_t color;
    std::string text;
};

class CChatWindow {
public:
    CChatWindow();
    ~CChatWindow();

    void AddChatMessage(const char* sender, uint32_t color, const char* text);
    void AddInfoMessage(const char* fmt, ...);
    void Draw();

private:
    std::vector<ChatMessage> m_Messages;
};

extern CChatWindow* pChatWindow;

#endif
