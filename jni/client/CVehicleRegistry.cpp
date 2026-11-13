#include "CVehicleRegistry.h"

std::map<uint16_t, CVehicle*> CVehicleRegistry::m_Vehicles;
std::map<CVehicle*, uint16_t> CVehicleRegistry::m_Ids;
uint16_t CVehicleRegistry::m_NextId = 0;

uint16_t
CVehicleRegistry::Register(CVehicle* pVeh)
{
	if (pVeh == nullptr)
		return 0xFFFF;

	// Машина уже зарегистрирована — возвращаем прежний id, чтобы повторное
	// создание того же объекта не раздваивало запись.
	auto it = m_Ids.find(pVeh);
	if (it != m_Ids.end())
		return it->second;

	// Ищем свободный номер. 0xFFFF зарезервирован под «пешком», поэтому
	// диапазон заканчивается на 0xFFFE.
	while (m_Vehicles.find(m_NextId) != m_Vehicles.end() || m_NextId == 0xFFFF) {
		m_NextId++;
	}
	if (m_NextId > 0xFFFE) {
		// Свободных номеров больше нет — такую машину синхронизировать нечем.
		return 0xFFFF;
	}

	uint16_t netId = m_NextId++;
	m_Vehicles[netId] = pVeh;
	m_Ids[pVeh] = netId;
	return netId;
}

bool
CVehicleRegistry::RegisterWithId(CVehicle* pVeh, uint16_t netId)
{
	if (pVeh == nullptr || netId == 0xFFFF)
		return false;
	// Машина уже под этим номером — ничего не делаем.
	if (m_Vehicles.find(netId) != m_Vehicles.end())
		return false;
	m_Vehicles[netId] = pVeh;
	m_Ids[pVeh] = netId;
	return true;
}

CVehicle*
CVehicleRegistry::Find(uint16_t netId)
{
	if (netId == 0xFFFF)
		return nullptr;
	auto it = m_Vehicles.find(netId);
	if (it == m_Vehicles.end())
		return nullptr;
	return it->second;
}

void
CVehicleRegistry::Unregister(CVehicle* pVeh)
{
	if (pVeh == nullptr)
		return;
	auto it = m_Ids.find(pVeh);
	if (it == m_Ids.end())
		return;
	m_Vehicles.erase(it->second);
	m_Ids.erase(it);
}

uint16_t
CVehicleRegistry::GetNetId(CVehicle* pVeh)
{
	if (pVeh == nullptr)
		return 0xFFFF;
	auto it = m_Ids.find(pVeh);
	if (it == m_Ids.end())
		return 0xFFFF;
	return it->second;
}

void
CVehicleRegistry::Shutdown()
{
	m_Vehicles.clear();
	m_Ids.clear();
	m_NextId = 0;
}
