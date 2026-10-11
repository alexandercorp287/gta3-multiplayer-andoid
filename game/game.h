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
