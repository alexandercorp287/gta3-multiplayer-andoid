#ifndef CDIALOGWINDOW_H
#define CDIALOGWINDOW_H

#include <cstdint>

#define DIALOG_STYLE_MSGBOX     0
#define DIALOG_STYLE_INPUT      1
#define DIALOG_STYLE_LIST       2
#define DIALOG_STYLE_PASSWORD   3

class CDialogWindow {
public:
    CDialogWindow();
    ~CDialogWindow();

    void ShowDialog(uint16_t wDialogID, uint8_t byteStyle, const char* title, const char* info, const char* button1, const char* button2);
    void Render();
    void Show(bool bShow) { m_bIsActive = bShow; }
    void Clear();

private:
    bool m_bIsActive;
    uint16_t m_wDialogID;
    uint8_t m_byteDialogStyle;
    char m_szTitle[128];
    char m_szInfo[2048];
    char m_szButton1[64];
    char m_szButton2[64];
    char m_szInput[128];
    int m_iSelectedListItem;
};

extern CDialogWindow* pDialogWindow;

#endif
