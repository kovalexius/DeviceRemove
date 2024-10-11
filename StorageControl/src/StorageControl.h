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

		bool m_isBlocked;	// �������� ����������
	};

	///
	/// \brief ��������� ��� ���������� � �������
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
		/// \brief ������� ������� ������������ ������ ����������, ����� ���������� ����� ������������ ����� usb ��������
		std::function<void(Device& _device)> m_newDeviceCallback;

		/// \brief ������� ������ �� ����������� �����������/������� ���������.
		/// ������ ������������� ��� ��������� ��������� ������� ��. ����� run()
		std::deque<DevInst> m_devQueue;
		/// ������� ��� ���� �������
		std::mutex	m_devQMutex;

	private:

		/// \breaf ����������� � ������ ����� ������� ��������� �����
		void init() override;

		/// \breaf ����������� � ������ ����� ���������� ��������� �����
		void unInit() override;

		/// \breaf ����� �������� ����� �������
		uint32_t getSleepTimeout() const override;

		/// \breaf ��������� ������� �����������/������� ��������� � ���������� ������.
		/// ��� ������������, ����� ������ ������������� ������ ������������ USB ��������, � ����� ����� ���������
		/// �������� ������� ������������������ ������� � ������ ��������� ���������� ����� ������������ ����������.
		/// �� �� ����� ����� ����� ��������� � registerBlockedDevicesEvent() �.�. ���������� �������� ��� ������������.
		bool run() override;

		/// \brief ���������� �� registerPlugEvent()
		/// ������������ �������� ���������� device event'��
		/// �������� ������ ������ ��� ���� ����� ���������� "this"
		/// � ������ ����������� ����������� ����������� ����� ������ � ������������ ������ � registerPlugEvent()
		void registerDevicesEvent();

	public:
		StorageControl();
		~StorageControl();

		///
		/// @brief �������� ������ ������� �����������
		/// 
		static std::pair<bool, std::vector<Device>> getDevices();

		///
		/// @brief ������������� ����������
		/// @return ���������� true, ���� ���������� ���� ������� ������� � �������������
		/// 
		static bool blockDevice(const Device& _device);

		///
		/// @brief �������������� ����������
		/// @return ���������� true, ���� ���������� ���� ������� ��������������
		/// 
		static bool unblockDevice(const Device& _device);

		///
		/// @brief ��������� ������� �� ���������� � ������� ������ ���������.
		/// ����� �������� ���������, ������ ��� �� ����� ���������� �� �������, 
		/// �� ���������� ������ ������ - �� winApi ��������, � ������� ������������ winapi ����.
		/// ���� ������������ ���������� ���� ����� � �������������� pimpl �� ���� ����� �������.
		/// 
		void addDevice2Queue(const DevInst& _device);

		/// 
		/// @brief ������������ ���������� ����� ����������� USB ���������, �������� �������
		void registerPlugEvent(std::function<void(Device& _device)> _callback);
	};
}