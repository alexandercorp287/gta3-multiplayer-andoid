#include <imgui.h>
#include "../core/Re3Log.h"
#include <vector>
#include <string>
#include <cmath>
#include <cstring>
#include <android/log.h>

#define LOG_TAG "KEYBOARD"
#define Log(...) RE3_LOGI(LOG_TAG, __VA_ARGS__)

#include "../gui/gui.h"
#include "keyboard.h"

#define TOUCH_PUSH 1
#define TOUCH_POP 2
#define TOUCH_MOVE 3

CKeyBoard* pKeyBoard = nullptr;

void cp1251_to_utf8(char *out, const char *in)
{
    if (!out || !in) return;

    const unsigned char *p = (const unsigned char *)in;
    while (*p) {
        if (*p < 128) {
            *out++ = *p++;
        } else if (*p >= 192 && *p <= 255) { // Russian CP1251 range
            unsigned int unicode = *p + 0x350;
            *out++ = (char)(0xc0 | (unicode >> 6));
            *out++ = (char)(0x80 | (unicode & 0x3f));
            p++;
        } else {
            *out++ = *p++;
        }
    }
    *out = '\0';
}

CKeyBoard::CKeyBoard()
{
    Log("Initializing Modern On-Screen KeyBoard..");
    m_bEnable = false;
    m_bInited = false;
    m_iLayout = LAYOUT_ENG;
    m_iCase = LOWER_CASE;
    m_iPushedKey = -1;
    m_utf8Input[0] = '\0';
    m_iInputOffset = 0;
    m_pHandler = nullptr;
}

CKeyBoard::~CKeyBoard()
{
}

void CKeyBoard::Open(keyboard_callback* handler)
{
    if (handler == nullptr) return;
    Close();
    m_pHandler = handler;
    m_bEnable = true;
}

void CKeyBoard::Close()
{
    m_bEnable = false;
    m_sInput.clear();
    m_iInputOffset = 0;
    m_utf8Input[0] = 0;
    m_iCase = LOWER_CASE;
    m_iPushedKey = -1;
    m_pHandler = nullptr;
}

bool CKeyBoard::OnTouchEvent(int type, bool multi, int x, int y)
{
    static bool bWannaClose = false;
    if (!m_bEnable) return true;

    if (type == TOUCH_PUSH && (float)y < m_Pos.y) bWannaClose = true;
    if (type == TOUCH_POP && (float)y < m_Pos.y && bWannaClose)
    {
        bWannaClose = false;
        Close();
        return false;
    }

    m_iPushedKey = -1;
    kbKey* key = GetKeyFromPos(x, y);
    if (!key) return false;

    switch (type)
    {
        case TOUCH_PUSH:
        case TOUCH_MOVE:
            m_iPushedKey = key->id;
            break;

        case TOUCH_POP:
            HandleInput(*key);
            break;
    }
    return false;
}

void CKeyBoard::HandleInput(kbKey &key)
{
    switch (key.type)
    {
        case KEY_DEFAULT:
        case KEY_SPACE:
            AddCharToInput(key.code[m_iCase]);
            break;

        case KEY_SWITCH:
            m_iLayout++;
            if (m_iLayout >= 3) m_iLayout = 0;
            m_iCase = LOWER_CASE;
            break;

        case KEY_BACKSPACE:
            DeleteCharFromInput();
            break;

        case KEY_SHIFT:
            m_iCase ^= 1;
            break;

        case KEY_SEND:
            Send();
            break;
    }
}

void CKeyBoard::AddCharToInput(char sym)
{
    if (m_sInput.length() < MAX_INPUT_LEN && sym)
    {
        m_sInput.push_back(sym);
        cp125_ret:
        cp1251_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset]);

        ImVec2 textSize = pGUI->GetFont()->CalcTextSizeA(m_fFontSize, FLT_MAX, 0.0f, m_utf8Input, nullptr, nullptr);
        if (textSize.x >= (m_Size.x - (m_Size.x * 0.04f)))
        {
            m_iInputOffset++;
            goto cp125_ret;
        }
    }
}

void CKeyBoard::DeleteCharFromInput()
{
    if (m_sInput.empty()) return;
    m_sInput.pop_back();

    while (m_iInputOffset > 0)
    {
        cp1251_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset - 1]);
        ImVec2 textSize = pGUI->GetFont()->CalcTextSizeA(m_fFontSize, FLT_MAX, 0.0f, m_utf8Input, nullptr, nullptr);

        if (textSize.x <= (m_Size.x - (m_Size.x * 0.04f)))
        {
            m_iInputOffset--;
        }
        else break;
    }
    cp1251_to_utf8(m_utf8Input, &m_sInput.c_str()[m_iInputOffset]);
}

void CKeyBoard::Send()
{
    if (m_pHandler) m_pHandler(m_sInput.c_str());
    m_bEnable = false;
}

kbKey* CKeyBoard::GetKeyFromPos(int x, int y)
{
    float fx = (float)x;
    float fy = (float)y;
    if (fx < m_Pos.x || fx > m_Pos.x + m_Size.x || fy < m_Pos.y + m_fKeySizeY || fy > m_Pos.y + m_Size.y)
        return nullptr;

    int iRow = (int)((fy - (m_Pos.y + m_fKeySizeY)) / m_fKeySizeY);
    if (iRow < 0 || iRow >= 5) return nullptr;

    for (auto &key : m_Rows[m_iLayout][iRow])
    {
        if (fx >= key.pos.x && fx <= (key.pos.x + key.width))
            return &key;
    }
    return nullptr;
}

void CKeyBoard::Render()
{
    if (!m_bEnable) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x <= 0 || io.DisplaySize.y <= 0) return;

    if (!m_bInited) {
        m_Size = ImVec2(io.DisplaySize.x * 0.85f, io.DisplaySize.y * 0.42f);
        m_Pos = ImVec2(io.DisplaySize.x * 0.075f, io.DisplaySize.y * (1.0f - 0.42f));
        m_fFontSize = pGUI->ScaleY(52.0f);
        m_fKeySizeY = m_Size.y / 6.0f;

        InitENG();
        InitRU();
        InitNUM();
        m_bInited = true;
    }

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // 1. Dark Modern Background with rounded corners
    drawList->AddRectFilled(m_Pos, ImVec2(m_Pos.x + m_Size.x, m_Pos.y + m_Size.y), ImColor(16, 16, 26, 240), 12.0f);
    drawList->AddRect(m_Pos, ImVec2(m_Pos.x + m_Size.x, m_Pos.y + m_Size.y), ImColor(80, 80, 120, 180), 12.0f, 0, 2.0f);

    // 2. Text Input Preview Box
    ImVec2 inputBoxStart = ImVec2(m_Pos.x + m_Size.x * 0.02f, m_Pos.y + m_fKeySizeY * 0.12f);
    ImVec2 inputBoxEnd = ImVec2(m_Pos.x + m_Size.x * 0.98f, m_Pos.y + m_fKeySizeY * 0.88f);
    drawList->AddRectFilled(inputBoxStart, inputBoxEnd, ImColor(28, 28, 45, 255), 6.0f);
    drawList->AddRect(inputBoxStart, inputBoxEnd, ImColor(90, 80, 140, 200), 6.0f, 0, 1.5f);

    char displayStr[512];
    snprintf(displayStr, sizeof(displayStr), "%s|", m_utf8Input[0] ? m_utf8Input : "");
    drawList->AddText(pGUI->GetFont(), m_fFontSize * 0.85f,
        ImVec2(inputBoxStart.x + 10.0f, inputBoxStart.y + m_fKeySizeY * 0.10f),
        ImColor(255, 255, 255, 255), displayStr);

    float fKeySizeY = m_fKeySizeY;

    // 3. Render Keys
    for (int i = 0; i < 5; i++)
    {
        for (auto &key : m_Rows[m_iLayout][i])
        {
            bool isPushed = (key.id == m_iPushedKey);

            ImU32 keyBgColor = isPushed ? ImColor(120, 60, 180, 255) :
                (key.type == KEY_DEFAULT ? ImColor(32, 32, 48, 230) : ImColor(55, 45, 80, 240));

            ImU32 keyBorderColor = isPushed ? ImColor(200, 120, 255, 255) : ImColor(70, 70, 100, 120);

            if (key.type != KEY_SPACE) {
                drawList->AddRectFilled(key.pos, ImVec2(key.pos.x + key.width, key.pos.y + fKeySizeY), keyBgColor, 6.0f);
                drawList->AddRect(key.pos, ImVec2(key.pos.x + key.width, key.pos.y + fKeySizeY), keyBorderColor, 6.0f, 0, 1.0f);
            }

            switch (key.type)
            {
                case KEY_DEFAULT:
                    drawList->AddText(pGUI->GetFont(), m_fFontSize * 0.8f, key.symPos, ImColor(240, 240, 250, 255), key.name[m_iCase]);
                    break;

                case KEY_SHIFT:
                {
                    ImU32 shiftColor = (m_iCase == UPPER_CASE) ? ImColor(160, 90, 255, 255) : ImColor(180, 180, 190, 220);
                    drawList->AddTriangleFilled(
                        ImVec2(key.pos.x + key.width * 0.37f, key.pos.y + fKeySizeY * 0.50f),
                        ImVec2(key.pos.x + key.width * 0.50f, key.pos.y + fKeySizeY * 0.25f),
                        ImVec2(key.pos.x + key.width * 0.63f, key.pos.y + fKeySizeY * 0.50f),
                        shiftColor);
                    drawList->AddRectFilled(
                        ImVec2(key.pos.x + key.width * 0.44f, key.pos.y + fKeySizeY * 0.5f - 1.0f),
                        ImVec2(key.pos.x + key.width * 0.56f, key.pos.y + fKeySizeY * 0.68f),
                        shiftColor);
                }
                break;

                case KEY_BACKSPACE:
                {
                    static ImVec2 points[5];
                    points[0] = ImVec2(key.pos.x + key.width * 0.35f, key.pos.y + fKeySizeY * 0.5f);
                    points[1] = ImVec2(key.pos.x + key.width * 0.45f, key.pos.y + fKeySizeY * 0.25f);
                    points[2] = ImVec2(key.pos.x + key.width * 0.65f, key.pos.y + fKeySizeY * 0.25f);
                    points[3] = ImVec2(key.pos.x + key.width * 0.65f, key.pos.y + fKeySizeY * 0.65f);
                    points[4] = ImVec2(key.pos.x + key.width * 0.45f, key.pos.y + fKeySizeY * 0.65f);
                    drawList->AddConvexPolyFilled(points, 5, ImColor(220, 220, 230, 255));
                }
                break;

                case KEY_SWITCH:
                {
                    const char* lNames[] = { "ENG", "RUS", "123" };
                    drawList->AddText(pGUI->GetFont(), m_fFontSize * 0.65f, key.symPos, ImColor(255, 255, 255, 255), lNames[m_iLayout]);
                }
                break;

                case KEY_SPACE:
                    drawList->AddRectFilled(
                        ImVec2(key.pos.x + key.width * 0.05f, key.pos.y + fKeySizeY * 0.25f),
                        ImVec2(key.pos.x + key.width * 0.95f, key.pos.y + fKeySizeY * 0.75f),
                        isPushed ? ImColor(120, 60, 180, 255) : ImColor(45, 35, 65, 240), 6.0f);
                    break;

                case KEY_SEND:
                    drawList->AddText(pGUI->GetFont(), m_fFontSize * 0.65f, key.symPos, ImColor(100, 255, 120, 255), "OK");
                    break;
            }
        }
    }
}

// ------------------- Keyboards Layout Definitions -------------------

static void AddKey(std::vector<kbKey> &row, int type, const char* nameLower, const char* nameUpper, char codeLower, char codeUpper, float width, ImVec2 pos, int id, float fontSize)
{
    kbKey key;
    key.type = type;
    key.width = width;
    key.pos = pos;
    key.id = id;
    key.code[0] = codeLower;
    key.code[1] = codeUpper;
    strncpy(key.name[0], nameLower, 31);
    strncpy(key.name[1], nameUpper, 31);

    ImVec2 sz = pGUI->GetFont()->CalcTextSizeA(fontSize * 0.8f, FLT_MAX, 0.0f, key.name[0]);
    key.symPos = ImVec2(pos.x + (width - sz.x) * 0.5f, pos.y + (fontSize * 0.15f));
    row.push_back(key);
}

void CKeyBoard::InitENG()
{
    int keyId = 0;
    float rowY = m_Pos.y + m_fKeySizeY;

    // Row 1 (10 keys)
    const char* r1Lower[] = {"q","w","e","r","t","y","u","i","o","p"};
    const char* r1Upper[] = {"Q","W","E","R","T","Y","U","I","O","P"};
    float w1 = m_Size.x / 10.0f;
    for(int i=0; i<10; i++) {
        AddKey(m_Rows[LAYOUT_ENG][0], KEY_DEFAULT, r1Lower[i], r1Upper[i], r1Lower[i][0], r1Upper[i][0], w1, ImVec2(m_Pos.x + i*w1, rowY), keyId++, m_fFontSize);
    }

    // Row 2 (9 keys)
    rowY += m_fKeySizeY;
    const char* r2Lower[] = {"a","s","d","f","g","h","j","k","l"};
    const char* r2Upper[] = {"A","S","D","F","G","H","J","K","L"};
    float w2 = m_Size.x / 9.0f;
    for(int i=0; i<9; i++) {
        AddKey(m_Rows[LAYOUT_ENG][1], KEY_DEFAULT, r2Lower[i], r2Upper[i], r2Lower[i][0], r2Upper[i][0], w2, ImVec2(m_Pos.x + i*w2, rowY), keyId++, m_fFontSize);
    }

    // Row 3 (Shift + 7 keys + Backspace)
    rowY += m_fKeySizeY;
    float w3 = m_Size.x / 9.0f;
    AddKey(m_Rows[LAYOUT_ENG][2], KEY_SHIFT, "^", "^", 0, 0, w3, ImVec2(m_Pos.x, rowY), keyId++, m_fFontSize);
    const char* r3Lower[] = {"z","x","c","v","b","n","m"};
    const char* r3Upper[] = {"Z","X","C","V","B","N","M"};
    for(int i=0; i<7; i++) {
        AddKey(m_Rows[LAYOUT_ENG][2], KEY_DEFAULT, r3Lower[i], r3Upper[i], r3Lower[i][0], r3Upper[i][0], w3, ImVec2(m_Pos.x + (i+1)*w3, rowY), keyId++, m_fFontSize);
    }
    AddKey(m_Rows[LAYOUT_ENG][2], KEY_BACKSPACE, "BS", "BS", 0, 0, w3, ImVec2(m_Pos.x + 8*w3, rowY), keyId++, m_fFontSize);

    // Row 4 (Switch + Space + OK)
    rowY += m_fKeySizeY;
    float w4_1 = m_Size.x * 0.20f;
    float w4_2 = m_Size.x * 0.60f;
    float w4_3 = m_Size.x * 0.20f;
    AddKey(m_Rows[LAYOUT_ENG][3], KEY_SWITCH, "ENG", "ENG", 0, 0, w4_1, ImVec2(m_Pos.x, rowY), keyId++, m_fFontSize);
    AddKey(m_Rows[LAYOUT_ENG][3], KEY_SPACE, " ", " ", ' ', ' ', w4_2, ImVec2(m_Pos.x + w4_1, rowY), keyId++, m_fFontSize);
    AddKey(m_Rows[LAYOUT_ENG][3], KEY_SEND, "OK", "OK", 0, 0, w4_3, ImVec2(m_Pos.x + w4_1 + w4_2, rowY), keyId++, m_fFontSize);
}

void CKeyBoard::InitRU()
{
    int keyId = 100;
    float rowY = m_Pos.y + m_fKeySizeY;

    // Row 1 (12 keys)
    const char* r1Lower[] = {"й","ц","у","к","е","н","г","ш","щ","з","х","ъ"};
    const char* r1Upper[] = {"Й","Ц","У","К","Е","Н","Г","Ш","Щ","З","Х","Ъ"};
    const unsigned char r1CodesLower[] = {0xE9,0xF6,0xF3,0xEA,0xE5,0xED,0xE3,0xF8,0xF9,0xE7,0xF5,0xFA};
    const unsigned char r1CodesUpper[] = {0xC9,0xD6,0xD3,0xCA,0xC5,0xCD,0xC3,0xD8,0xD9,0xC7,0xD5,0xDA};
    float w1 = m_Size.x / 12.0f;
    for(int i=0; i<12; i++) {
        AddKey(m_Rows[LAYOUT_RUS][0], KEY_DEFAULT, r1Lower[i], r1Upper[i], (char)r1CodesLower[i], (char)r1CodesUpper[i], w1, ImVec2(m_Pos.x + i*w1, rowY), keyId++, m_fFontSize);
    }

    // Row 2 (11 keys)
    rowY += m_fKeySizeY;
    const char* r2Lower[] = {"ф","ы","в","а","п","р","о","л","д","ж","э"};
    const char* r2Upper[] = {"Ф","Ы","В","А","П","Р","О","Л","Д","Ж","Э"};
    const unsigned char r2CodesLower[] = {0xF4,0xFB,0xE2,0xE0,0xEF,0xF0,0xEE,0xEB,0xE4,0xE6,0xFD};
    const unsigned char r2CodesUpper[] = {0xD4,0xDB,0xC2,0xC0,0xCF,0xD0,0xCE,0xCB,0xC4,0xC6,0xDD};
    float w2 = m_Size.x / 11.0f;
    for(int i=0; i<11; i++) {
        AddKey(m_Rows[LAYOUT_RUS][1], KEY_DEFAULT, r2Lower[i], r2Upper[i], (char)r2CodesLower[i], (char)r2CodesUpper[i], w2, ImVec2(m_Pos.x + i*w2, rowY), keyId++, m_fFontSize);
    }

    // Row 3 (Shift + 9 keys + Backspace)
    rowY += m_fKeySizeY;
    float w3 = m_Size.x / 11.0f;
    AddKey(m_Rows[LAYOUT_RUS][2], KEY_SHIFT, "^", "^", 0, 0, w3, ImVec2(m_Pos.x, rowY), keyId++, m_fFontSize);
    const char* r3Lower[] = {"я","ч","с","м","и","т","ь","б","ю"};
    const char* r3Upper[] = {"Я","Ч","С","М","И","Т","Ь","Б","Ю"};
    const unsigned char r3CodesLower[] = {0xFF,0xF7,0xF1,0xEC,0xE8,0xF2,0xFC,0xE1,0xFE};
    const unsigned char r3CodesUpper[] = {0xDF,0xD7,0xD1,0xCC,0xC8,0xD2,0xDC,0xC1,0xDE};
    for(int i=0; i<9; i++) {
        AddKey(m_Rows[LAYOUT_RUS][2], KEY_DEFAULT, r3Lower[i], r3Upper[i], (char)r3CodesLower[i], (char)r3CodesUpper[i], w3, ImVec2(m_Pos.x + (i+1)*w3, rowY), keyId++, m_fFontSize);
    }
    AddKey(m_Rows[LAYOUT_RUS][2], KEY_BACKSPACE, "BS", "BS", 0, 0, w3, ImVec2(m_Pos.x + 10*w3, rowY), keyId++, m_fFontSize);

    // Row 4 (Switch + Space + OK)
    rowY += m_fKeySizeY;
    float w4_1 = m_Size.x * 0.20f;
    float w4_2 = m_Size.x * 0.60f;
    float w4_3 = m_Size.x * 0.20f;
    AddKey(m_Rows[LAYOUT_RUS][3], KEY_SWITCH, "RUS", "RUS", 0, 0, w4_1, ImVec2(m_Pos.x, rowY), keyId++, m_fFontSize);
    AddKey(m_Rows[LAYOUT_RUS][3], KEY_SPACE, " ", " ", ' ', ' ', w4_2, ImVec2(m_Pos.x + w4_1, rowY), keyId++, m_fFontSize);
    AddKey(m_Rows[LAYOUT_RUS][3], KEY_SEND, "OK", "OK", 0, 0, w4_3, ImVec2(m_Pos.x + w4_1 + w4_2, rowY), keyId++, m_fFontSize);
}

void CKeyBoard::InitNUM()
{
    int keyId = 200;
    float rowY = m_Pos.y + m_fKeySizeY;

    // Row 1 (10 numbers)
    const char* r1[] = {"1","2","3","4","5","6","7","8","9","0"};
    float w1 = m_Size.x / 10.0f;
    for(int i=0; i<10; i++) {
        AddKey(m_Rows[LAYOUT_NUM][0], KEY_DEFAULT, r1[i], r1[i], r1[i][0], r1[i][0], w1, ImVec2(m_Pos.x + i*w1, rowY), keyId++, m_fFontSize);
    }

    // Row 2 (10 symbols)
    rowY += m_fKeySizeY;
    const char* r2[] = {"-","/",":",";","(",")","$","&","@","\""};
    float w2 = m_Size.x / 10.0f;
    for(int i=0; i<10; i++) {
        AddKey(m_Rows[LAYOUT_NUM][1], KEY_DEFAULT, r2[i], r2[i], r2[i][0], r2[i][0], w2, ImVec2(m_Pos.x + i*w2, rowY), keyId++, m_fFontSize);
    }

    // Row 3 (5 symbols + Backspace)
    rowY += m_fKeySizeY;
    float w3 = m_Size.x / 8.0f;
    const char* r3[] = {".",",","?","!","'","_"};
    for(int i=0; i<6; i++) {
        AddKey(m_Rows[LAYOUT_NUM][2], KEY_DEFAULT, r3[i], r3[i], r3[i][0], r3[i][0], w3, ImVec2(m_Pos.x + i*w3, rowY), keyId++, m_fFontSize);
    }
    AddKey(m_Rows[LAYOUT_NUM][2], KEY_BACKSPACE, "BS", "BS", 0, 0, w3 * 2.0f, ImVec2(m_Pos.x + 6*w3, rowY), keyId++, m_fFontSize);

    // Row 4 (Switch + Space + OK)
    rowY += m_fKeySizeY;
    float w4_1 = m_Size.x * 0.20f;
    float w4_2 = m_Size.x * 0.60f;
    float w4_3 = m_Size.x * 0.20f;
    AddKey(m_Rows[LAYOUT_NUM][3], KEY_SWITCH, "123", "123", 0, 0, w4_1, ImVec2(m_Pos.x, rowY), keyId++, m_fFontSize);
    AddKey(m_Rows[LAYOUT_NUM][3], KEY_SPACE, " ", " ", ' ', ' ', w4_2, ImVec2(m_Pos.x + w4_1, rowY), keyId++, m_fFontSize);
    AddKey(m_Rows[LAYOUT_NUM][3], KEY_SEND, "OK", "OK", 0, 0, w4_3, ImVec2(m_Pos.x + w4_1 + w4_2, rowY), keyId++, m_fFontSize);
}
