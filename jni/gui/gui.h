#ifndef GUI_H
#define GUI_H

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"

class CGui {
public:
    CGui();
    ~CGui();

    void Init();
    void Render();
    void RenderMenuMultiplayerButton();

    // Масштабирование координат (нужно клавиатуре и виджетам)
    float ScaleX(float x);
    float ScaleY(float y);
    float GetFontSize();

    ImFont* GetFont() { return m_pFont; }
    void SetFont(ImFont* f) { m_pFont = f; }

    bool m_bInited;
    bool m_bShow;

    ImFont* m_pFont;
};

extern CGui* pGUI;

#endif
