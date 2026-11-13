#pragma once

#include <cstdint>

#define MENUACTION_STARTMULTIPLAYER        250

// Точные офсеты из IDA Pro для libR1.so
#define OFFSET_Render2dStuff               0x220800
#define OFFSET_CMenuManager_DrawFrontEnd   0x222170
#define OFFSET_CMenuManager_ProcessCommand 0x224B20
#define OFFSET_GetIconForOption            0x221464
#define OFFSET_ProcessButtonPresses        0x22C08C
#define OFFSET_CText_Get                   0x2FDDC0
#define OFFSET_FindPlayerPed               0x241BA0
#define OFFSET_CWorld_Add                  0x256C34
#define OFFSET_CWorld_Remove               0x256A04
#define OFFSET_SetModelIndex               0x214C7C
#define OFFSET_RequestModel                0x2E232C
#define OFFSET_GiveWeapon                  0x27CE80
#define OFFSET_CEntity_ProcessControl      0x19DB0C
#define OFFSET_CEntity_ProcessCollision    0x19DB10
#define OFFSET_CEntity_ProcessShift        0x19DB14
#define OFFSET_CEntity_Teleport            0x19DB18
#define OFFSET_CEntity_FlagToDestroy       0x19DB1C
#define OFFSET_GetIconForOption  		   0x221464
#define OFFSET_aScreens                    0x473974
#define OFFSET_DoSettingsBeforeStartingAGame  0x22E538
#define OFFSET_CFileMgr_OpenFile              0x21BA30
#define OFFSET_CFileMgr_Seek                  0x21BB20
#define OFFSET_CFileMgr_Read                  0x21BAE8

// Наш пункт живёт в экране 51, слот 2 (проверено логом)
#define MENU_SCREEN_MAIN          51
#define MENU_SLOT_MULTIPLAYER     2

// Новые офсеты, сверенные по символам libR1.so (AARCH64)
#define OFFSET_AsciiToUnicode              0x2BCB44
#define OFFSET_CMenuManager_Process        0x22B64C
#define OFFSET_CMenuManager_Draw           0x223008
#define OFFSET_m_KeyPressedCode            0x4737B4
#define OFFSET_CMenuManager_Back           0x22E4A0

// CFont (сверено по символам libR1.so)
// ВАЖНО: адреса из .dynsym (реальный код), НЕ из PLT!
#define OFFSET_CFont_PrintString           0x2BDF80
#define OFFSET_CFont_SetScale              0x2BD1C8
#define OFFSET_CFont_SetColor              0x2BD1F8
#define OFFSET_CFont_SetFontStyle          0x2BD2F0
#define OFFSET_CFont_SetCentreOn           0x2BF9D8
#define OFFSET_CFont_SetCentreOff          0x2BD260


// ---- Таблица меню (по дизассемблеру ProcessButtonPresses 0x22C08C) ----
// menuTable = *(void**)(base + 0x46DE68)
// screen    = menuTable + screenIdx * 0x19C
// item      = screen + itemIdx * 0x14
// action    = *(uint32_t*)(item + 0x34)
#define OFFSET_g_MenuTablePtr              0x46DE68
#define MENU_SCREEN_STRIDE                 0x19C
#define MENU_ITEM_STRIDE                   0x14
#define MENU_ITEM_ACTION_OFF               0x34
#define MENU_ITEM_TEXT_OFF                 0x00
#define MENU_ITEM_ICON_OFF                 0x08
#define MENU_ACTION_STARTMULTIPLAYER       250


struct Vector3Raw {
    float x, y, z;
};

// Структуры элементов Главного Меню CMenuManager
struct CMenuEntry {
    int32_t m_nAction;
    char    m_szName[8];
    int32_t m_nSaveSlot;
    int32_t m_nTargetMenu;
};

struct CMenuScreen {
    char        m_szTitle[8];
    int32_t     m_nScreenType;
    CMenuEntry  m_aEntries[12];
    int32_t     m_nNumEntries;
};

// Сигнатуры типов C++ функций
typedef void* (*FindPlayerPed_t)(int id);
typedef void (*CWorld_Add_t)(void* entity);
typedef void (*CWorld_Remove_t)(void* entity);
typedef void (*SetModelIndex_t)(void* entity, unsigned int modelIndex);
typedef void (*RequestModel_t)(int modelIndex, int flags);
typedef void (*EntityTeleport_t)(void* entity, Vector3Raw pos);
typedef void (*DrawFrontEnd_t)(void* menuMgr);
typedef void (*ProcessCommand_t)(void* menuMgr, int action);
typedef void* (*GetIconForOption_t)(const char* optionName);
typedef void (*ProcessButtonPresses_t)(void* this_ptr);
typedef uint16_t* (*CText_Get_t)(void* thiz, const char* key);
// Глобальные вызовы функций игры
void* GetLocalPlayerPed();
void WorldAddEntity(void* entity);
void WorldRemoveEntity(void* entity);
void EntitySetModelIndex(void* entity, unsigned int modelIndex);
void RequestGameModel(int modelIndex, int flags);
void EntityTeleport(void* entity, float x, float y, float z);
