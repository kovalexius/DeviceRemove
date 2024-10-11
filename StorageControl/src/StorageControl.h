#pragma once

#include <vector>
#include <string>
#include <deque>
#include <functional>

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

	///
	/// \brief структура для добавления в очередь
	///
	enum Actions
	{
		ACTION_DEVICEINSTANCEENUMERATED,
		ACTION_DEVICEINSTANCESTARTED
	};
	struct DevInst
	{
		std::string m_instanceId;
		Actions		m_action;
	};

	bool getDiData(const sudis::base::HDevInfo& hDevInfo, const Device& _device, SP_DEVINFO_DATA& _out);

	void testGetProperties(const Device& _device);

	class StorageControl : sudis::base::Thread<StorageControl>
	{
	private:
		/// \brief Функтор который регистрирует клиент библиотеки, будет вызываться когда подключается новый usb носитель
		std::function<void(Device& _device)> m_newDeviceCallback;

		/// \brief Очередь данных от обработчика вставленных/вынутых устройств.
		/// Очердь предназначена для обработки отдельным потоком см. метод run()
		std::deque<DevInst> m_devQueue;
		/// Мьютекс для этой очереди
		std::mutex	m_devQMutex;

	private:

		/// \breaf выполняется в потоке перед началом основного цикла
		void init() override;

		/// \breaf выполняется в потоке после завершения основного цикла
		void unInit() override;

		/// \breaf время ожидания между циклами
		uint32_t getSleepTimeout() const override;

		/// \breaf Обработка очереди вставленных/вынутых устройств в отдельнолм потоке.
		/// тут определяется, какие данные соответствуют новому вставленному USB носителю, а какие можно отбросить
		/// Вызывает заранее зарегистрированный функтор в случае успешного опредления вновь вставленного устройства.
		/// Мы не можем сразу вести обработку в registerBlockedDevicesEvent() т.к. длительные операции там нежелательны.
		bool run() override;

		/// \brief вызывается из registerPlugEvent()
		/// регистрирует виндовый обработчик device event'ов
		/// является членом класса для того чтобы пробросить "this"
		/// В итогах рассмотреть возможность выпиливания этого метода с объединением логики с registerPlugEvent()
		void registerDevicesEvent();

	public:
		StorageControl();
		~StorageControl();

		///
		/// @brief Получить список внешних накопителей
		/// 
		static std::pair<bool, std::vector<Device>> getDevices();

		///
		/// @brief Заблокировать устройство
		/// @return возвращает true, если устройство было успешно найдено и заблокировано
		/// 
		static bool blockDevice(const Device& _device);

		///
		/// @brief Разблокировать устройство
		/// @return возвращает true, если устройство было успешно разблокировано
		/// 
		static bool unblockDevice(const Device& _device);

		///
		/// @brief добавляет событие об устройстве в очередь потоко безопасно.
		/// метод является публичным, потому что он будет вызываться из функции, 
		/// не являющейся членом класса - из winApi коллбека, в котором присутствуют winapi типы.
		/// Если впоследствии переделать этот класс с использованием pimpl то этот метод удалить.
		/// 
		void addDevice2Queue(const DevInst& _device);

		/// 
		/// @brief Регистрирует обработчик вновь вставленных USB носителей, принимая функтор
		void registerPlugEvent(std::function<void(Device& _device)> _callback);
	};
}