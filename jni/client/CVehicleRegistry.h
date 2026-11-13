#pragma once
#include "common.h"
#include <map>

class CVehicle;

// Реестр сетевых машин.
//
// Проблема, которую он решает: раньше локальный игрок отправлял в синке
// vehicleId = индекс машины в СВОЁМ пуле (CPools::GetVehiclePool()). Клиент
// напротив брал этот индекс и лез с ним в СВОЙ пул. Пулы у клиентов разные,
// поэтому индекс почти всегда указывал на другую машину или на свободный
// слот: пед либо не садился никуда, либо пропадал внутри чужой машины
// («когда в тачке сидишь — тебя не видно»).
//
// Теперь сервер при создании машины (ID_CREATE_VEHICLE) не присылает id, но
// пакет RELIABLE_ORDERED, поэтому все клиенты получают машины в одном и том
// же порядке. Присваиваем каждой машине порядковый номер и шлём в синке
// именно его — он одинаков на всех клиентах.
class CVehicleRegistry {
public:
    // Машина создана из сети: выдаём следующий свободный сетевой id.
    // Возвращает id, под которым машина зарегистрирована.
    static uint16_t Register(CVehicle* pVeh);

    // Зарегистрировать машину под номером, который прислал сервер.
    // Так номера совпадают с серверными (ID_CREATE_VEHICLE). Если номер занят
    // или 0xFFFF, возвращает false.
    static bool RegisterWithId(CVehicle* pVeh, uint16_t netId);

    // Найти машину по сетевому id. nullptr, если машина уже удалена.
    static CVehicle* Find(uint16_t netId);

    // Машина уничтожена (взрыв/удаление из мира).
    static void Unregister(CVehicle* pVeh);

    // Сетевой id для уже известной машины. 0xFFFF, если машина не из сети.
    static uint16_t GetNetId(CVehicle* pVeh);

    static void Shutdown();

private:
    static std::map<uint16_t, CVehicle*> m_Vehicles; // netId -> машина
    static std::map<CVehicle*, uint16_t> m_Ids;      // машина -> netId
    static uint16_t m_NextId;
};
