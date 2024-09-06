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
		/// @brief �������� ������ ������� �����������
		/// @param _isBlocked ���� true, �� ������ ��� ��������������� ����������
		/// 
		std::vector<Device> getDevices(bool _isBlocked);

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
	};
}