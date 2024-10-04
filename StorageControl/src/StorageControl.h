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

		bool m_isBlocked;	// �������� ����������
	};

	class StorageControl : sudis::base::Thread<StorageControl>
	{
		/// \breaf ����������� � ������ ����� ������� ��������� �����
		void init() override;

		/// \breaf ����������� � ������ ����� ���������� ��������� �����
		void unInit() override;

		/// \breaf ����� �������� ����� �������
		uint32_t getSleepTimeout() const override;

		/// \breaf �������� ���� ����������� ��������
		/// \return false, ���� ���������� ����� �������� ��������� ����, ����� - ����� ��������
		bool run() override;

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