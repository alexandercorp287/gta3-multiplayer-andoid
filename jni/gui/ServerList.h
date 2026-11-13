#ifndef SERVERLIST_H
#define SERVERLIST_H

// Окно выбора сервера: открывается по тапу на пункт MULTIPLAYER.
// Соединение (и запуск игры) запускается только кнопкой Connect.
void ServerList_Render();
bool ServerList_IsOpen();
void ServerList_Close();
void ServerList_Open();

#endif
