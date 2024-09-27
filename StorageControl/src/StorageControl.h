#pragma once

#include <vector>
#include <string>

#include "RaiiWrapper.h"

#ifdef _WIN32	// ѕотом это убрать
#include <Windows.h>
#include <SetupAPI.h>
#endif

namespace sudis::storage_control
{
	struct Device
	{
		std::string m_vid;
		std::string m_pid;
		std::string m_serial;
		std::string m_product;
		std::string m_vendor;

		bool m_isBlocked;	// ѕризнако блокировки
#ifdef _WIN32	// ѕотом это убрать
		SP_DEVINFO_DATA m_diData;
#endif
	};

	class StorageControl
	{
#ifdef _WIN32	// ”брать это в детали реализации
		HDEVINFO m_deviceInfoHandle;
		const GUID* m_classGuid;
		void enableDevice(const Device& _device, const bool _enable);
#endif
	public:
		StorageControl();
		~StorageControl();

		///
		/// @brief ѕолучить список внешних накопителей
		/// @param _isBlocked если true, то учесть уже заблокированные устройства
		/// 
		std::pair<bool, std::vector<Device>> getDevices(bool _isBlocked);

		///
		/// @brief «аблокировать устройство
		/// @return возвращает true, если устройство было успешно найдено и заблокировано
		/// 
		bool blockDevice(const Device& _device);

		///
		/// @brief –азблокировать устройство
		/// @return возвращает true, если устройство было успешно разблокировано
		/// 
		bool unblockDevice(const Device& _device);

		void registerPlugEvent();
	};
}