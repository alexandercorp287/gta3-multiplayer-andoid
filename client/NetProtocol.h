#ifndef NETPROTOCOL_H
#define NETPROTOCOL_H

#include <cstdint>

#pragma pack(push, 1)

struct Vector3 {
    float x, y, z;
};

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

    ID_SET_CAMERA_POS = 240,
    ID_SET_CAMERA_LOOKAT = 241,
    ID_SET_CAMERA_BEHIND = 242,
    ID_SET_CAMERA_FADE = 243,
    ID_SET_CAMERA_SHAKE = 244,
    ID_RESTORE_CAMERA = 245,
    ID_SHOW_DIALOG_VMP = 246,
    ID_DIALOG_RESPONSE_VMP = 247,

    ID_CREATE_ACTOR = 250,
    ID_DESTROY_ACTOR = 251,
    ID_SET_ACTOR_POS = 252,
    ID_SET_ACTOR_HEALTH = 253,
    ID_SET_ACTOR_ANIM = 254
};

struct OnFootSyncData {
    uint16_t lrAnalog;
    uint16_t udAnalog;
    uint16_t keys;
    Vector3  pos;
    float    rotation;
    uint8_t  health;
    uint8_t  armour;
    uint8_t  weapon;
    uint8_t  specialAction;
    uint16_t vehicleId;
    uint32_t virtualWorld;
    Vector3  moveSpeed;
    Vector3  camPos;
    Vector3  camFront;
};

#pragma pack(pop)

#endif
