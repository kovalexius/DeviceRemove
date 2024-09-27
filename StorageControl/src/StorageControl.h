#pragma once

#include <vector>
#include <string>

#include "RaiiWrapper.h"

#ifdef _WIN32	// ����� ��� ������
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

		bool m_isBlocked;	// �������� ����������
#ifdef _WIN32	// ����� ��� ������
		SP_DEVINFO_DATA m_diData;
#endif
	};

	class StorageControl
	{
#ifdef _WIN32	// ������ ��� � ������ ����������
		HDEVINFO m_deviceInfoHandle;
		const GUID* m_classGuid;
		void enableDevice(const Device& _device, const bool _enable);
#endif
	public:
		StorageControl();
		~StorageControl();

		///
		/// @brief �������� ������ ������� �����������
		/// @param _isBlocked ���� true, �� ������ ��� ��������������� ����������
		/// 
		std::pair<bool, std::vector<Device>> getDevices(bool _isBlocked);

		///
		/// @brief ������������� ����������
		/// @return ���������� true, ���� ���������� ���� ������� ������� � �������������
		/// 
		bool blockDevice(const Device& _device);

		///
		/// @brief �������������� ����������
		/// @return ���������� true, ���� ���������� ���� ������� ��������������
		/// 
		bool unblockDevice(const Device& _device);

		void registerPlugEvent();
	};
}