#pragma once

#include <vector>
#include <string>

#include "RaiiWrapper.h"
#include "Thread.h"

namespace sudis::storage_control
{
	struct Device
	{
		Device() : m_isBlocked(false)
		{}
		std::string m_vid;
		std::string m_pid;
		std::string m_serial;
		std::string m_product;
		std::string m_vendor;

		bool m_isBlocked;	// Признако блокировки
	};

	class StorageControl : sudis::base::Thread<StorageControl>
	{
		/// \breaf выполняется в потоке перед началом основного цикла
		void init() override;

		/// \breaf выполняется в потоке после завершения основного цикла
		void unInit() override;

		/// \breaf время ожидания между циклами
		uint32_t getSleepTimeout() const override;

		/// \breaf основной цикл выполняемых операций
		/// \return false, если необходимо сразу начинать следующий цикл, иначе - после таймаута
		bool run() override;

	public:
		StorageControl();
		~StorageControl();

		///
		/// @brief Получить список внешних накопителей
		/// @param _isBlocked если true, то учесть уже заблокированные устройства
		/// 
		std::pair<bool, std::vector<Device>> getDevices(bool _isBlocked);

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

		void registerPlugEvent();
	};
}