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

    bool m_bInited;
};

extern CGui* pGUI;

#endif
