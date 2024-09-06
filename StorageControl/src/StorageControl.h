#pragma once

#include <vector>
#include <string>


namespace sudis::storage_control
{
	struct Device
	{
		std::string m_vid;
		std::string m_pid;
		std::string m_serial;
		std::string m_product;
	};
	class StorageControl
	{
	public:
		///
		/// @brief Получить список внешних накопителей
		/// @param _isBlocked если true, то учесть уже заблокированные устройства
		/// 
		std::vector<Device> getDevices(bool _isBlocked);

		///
		/// @brief Заблокировать устройство
		/// @return возвращает true, если устройство было успешно найдено и заблокировано
		/// 
		bool blockDevice(const Device& _device);

		///
		/// @brief Разблокировать устройство
		/// @return возвращает true, если устройство было успешно разблокировано
		/// 
		bool unblockDevice(const Device& _device);
	};
}