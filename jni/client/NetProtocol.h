#ifndef NETPROTOCOL_H
#define NETPROTOCOL_H

#include <cstdint>
#include <cstring>

// CVector из re3: три float подряд, без виртуальных функций
struct CVector {
    float x, y, z;
    CVector() : x(0), y(0), z(0) {}
    CVector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};
typedef CVector Vector3;

enum eNetPacketId {
    ID_PLAYER_SYNC_VMP = 213,
    ID_VEHICLE_SYNC_VMP = 214,
    ID_CHAT_MESSAGE_VMP = 215,
    ID_CONNECTION_SET_ID_VMP = 216,
    ID_NEW_PLAYER_VMP = 217,
    ID_PLAYER_QUIT_VMP = 218,
    ID_PLAYER_SPAWN_VMP = 219,
    ID_PLAYER_LOGIN_VMP = 220,
    ID_PLAYER_REQUEST_CLASS = 221,

    ID_SET_PLAYER_POS = 222,
    ID_SET_PLAYER_HEALTH = 223,
    ID_SET_PLAYER_ARMOUR = 224,
    ID_GIVE_PLAYER_WEAPON = 225,
    ID_SET_WORLD_TIME = 226,
    ID_SET_WORLD_WEATHER = 227,

    ID_SET_PLAYER_FACING_ANGLE = 228,
    ID_SET_PLAYER_SKIN = 229,
    ID_RESET_PLAYER_WEAPONS = 230,
    ID_SHOW_GAME_TEXT = 231,
    ID_KICK_PLAYER = 232,
    ID_SET_PLAYER_CASH = 233,
    ID_CREATE_VEHICLE = 234,
    ID_DESTROY_VEHICLE = 235,
    ID_PLAYER_REQUEST_SPAWN = 236,
    ID_FORCE_CLASS_SELECTION = 237,
    ID_SET_PLAYER_INTERIOR_VMP = 238,
    ID_PLAYER_DEATH_VMP = 239,
    ID_PLAYER_DAMAGE_VMP = 198,

    // КАМЕРА
    ID_SET_CAMERA_POS = 240,
    ID_SET_CAMERA_LOOKAT = 241,
    ID_SET_CAMERA_BEHIND = 242,
    ID_SET_CAMERA_FADE = 243,
    ID_SET_CAMERA_SHAKE = 244,
    ID_RESTORE_CAMERA = 245,
    ID_SHOW_DIALOG_VMP = 246,
    ID_DIALOG_RESPONSE_VMP = 247,
    ID_SET_WEATHER_VMP = 248,
    ID_SET_MARKERS_MODE_VMP = 249,

    // АКТЁРЫ (NPC)
    ID_CREATE_ACTOR = 250,
    ID_DESTROY_ACTOR = 251,
    ID_SET_ACTOR_POS = 252,
    ID_SET_ACTOR_HEALTH = 253,
    ID_SET_ACTOR_ANIM = 254,

    // ТРАНСПОРТНЫЙ УРОВЕНЬ (не данные игры)
    // Клиент раз в секунду шлёт RELIABLE-киперайв: ACK от сервера сбрасывает
    // таймер мёртвого соединения. Без этого UNRELIABLE-синка (30 мс) достаточно
    // одной потери в окне таймаута, чтобы RakNet объявил связь мёртвой.
    ID_MP_KEEPALIVE_VMP = 199,

    // ПИНГ ИГРОКОВ: сервер раз в 2 с шлёт всем список (id, ping).
    // Формат: [ID][count][ (uint8 id, uint16 ping) x count ]
    ID_PLAYER_PING_VMP = 200
};

#pragma pack(push, 1)

struct OnFootSyncData {
    uint16_t lrAnalog;
    uint16_t udAnalog;
    uint16_t wKeys;
    CVector  vecPos;
    float    fRotation; // Угол поворота (простой float)
    uint8_t  byteHealth;
    uint8_t  byteArmour;
    uint8_t  byteCurrentWeapon;
    uint8_t  byteSpecialAction;
    uint16_t vehicleId;
    uint32_t virtualWorld;
    CVector  vecMoveSpeed;
    CVector  camPos;
    CVector  camFront;
};

// ВАЖНО: раскладка этой структуры должна байт в байт совпадать с
// OnFootSyncData выше. Сервер читает пакет ID_PLAYER_SYNC_VMP именно как
// PlayerSyncData, а клиент его отправляет как OnFootSyncData. Если поля
// разойдутся, сервер прочитает moveSpeed/camPos со сдвигом в 6 байт
// (vehicleId + virtualWorld) и разошлёт остальным игрокам мусор вместо
// позиции и скорости.
struct PlayerSyncData {
    uint16_t lrAnalog;          // LeftStickX
    uint16_t udAnalog;          // LeftStickY
    uint16_t keys;              // битовая маска нажатий
    CVector  pos;               // позиция поги
    float    rotation;          // угол поворота
    uint8_t  health;            // здоровье
    uint8_t  armour;            // броня
    uint8_t  weapon;            // текущий слот оружия
    uint8_t  specialAction;     // спец. действие
    uint16_t vehicleId;         // 0xFFFF — пешком, иначе индекс машины
    uint32_t virtualWorld;      // интерьер/виртуальный мир
    CVector  moveSpeed;         // вектор скорости
    CVector  camPos;            // позиция камеры
    CVector  camFront;          // направление камеры
};

// Проверка на этапе компиляции: две структуры одного назначения не должны
// разъезжаться по размеру.
static_assert(sizeof(OnFootSyncData) == sizeof(PlayerSyncData),
              "OnFootSyncData и PlayerSyncData должны совпадать по размеру");

struct NewPlayerPacket {
    uint8_t bytePacketId;
    uint8_t nPlayerId;
    char szName[32];
};

#pragma pack(pop)

#endif
