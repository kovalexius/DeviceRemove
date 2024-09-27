#ifdef _WIN32
#include <iostream>
#include <sstream>
#include <iomanip>

#include "StorageControl.h"
#include "Str.h"

#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <usbiodef.h>
#include <usbioctl.h>
#include <cfgmgr32.h>

namespace sudis::storage_control
{
	template<typename T>
	std::string int_to_hex(T i, int _width)
	{
		std::stringstream stream;

		stream << std::setfill('0') << std::setw(_width) << std::hex << i;
		return stream.str();
	}

	std::string toUpper(const std::string& _str)
	{
		std::string result;
		for (auto& ch : _str)
		{
			result += std::toupper(ch);
		}
		return result;
	}


	/// 
	/// @brief Получить список конечных устройств с USB разветлителя
	/// @param _handle файл устройства разветвителя
	/// @param _diData эта структура тут не используется, а просто сохраняется в случае пополнения списка _devList
	/// 
	void getDeviceFromPort(const UINT ports, const HANDLE _handle, const SP_DEVINFO_DATA& _diData, std::vector<Device>& _devList)
	{
		for (int j = 1; j <= ports; j++)
		{
			USB_NODE_CONNECTION_INFORMATION_EX coninfo = { 0 };
			coninfo.ConnectionIndex = j;

			DWORD bytes_read = 0;
			if (!DeviceIoControl(_handle, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX, &coninfo, sizeof(coninfo), &coninfo, sizeof(coninfo), &bytes_read, 0))
			{
				std::cerr << "Failed IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX request by DeviceIoControl() from port: " << j << " errorCode: " << GetLastError() << std::endl;
				continue;
			}

			/// Под вопросом
			if (coninfo.ConnectionStatus == 0)
			{
				std::cout << "Not connected port " << j <<  std::endl << std::endl;
				continue; //нет устройства
			}

			Device device;

			std::cout << "Port " << j << 
				": USB v" << std::hex << (int)coninfo.DeviceDescriptor.bcdUSB << std::dec << " device" << 
				" connectionStatus: " << coninfo.ConnectionStatus << std::endl;
			std::cout << "VID: " << std::hex << (int)coninfo.DeviceDescriptor.idVendor << " PID: " << (int)coninfo.DeviceDescriptor.idProduct << std::dec << std::endl;

			device.m_vid = int_to_hex((int)coninfo.DeviceDescriptor.idVendor, 4);
			device.m_pid = int_to_hex((int)coninfo.DeviceDescriptor.idProduct, 4);

			const UINT BUFSIZE = 1000;
			char buffer[BUFSIZE] = { 0 };
			USB_DESCRIPTOR_REQUEST* req = (USB_DESCRIPTOR_REQUEST*)&buffer;

			/*Serial number*/
			req->ConnectionIndex = j;
			req->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8) + coninfo.DeviceDescriptor.iSerialNumber;
			req->SetupPacket.wLength = BUFSIZE - sizeof(USB_DESCRIPTOR_REQUEST);
			req->SetupPacket.wIndex = 0x409;			//US English
			if (!DeviceIoControl(_handle, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
				&buffer, sizeof(buffer), &buffer, sizeof(buffer), &bytes_read, 0))
			{
				std::cerr << "Failed iSerialNumber request by DeviceIoControl() from port: " << j << " errorCode: " << GetLastError() << std::endl;
				continue;
			}
			USB_STRING_DESCRIPTOR* desc = (USB_STRING_DESCRIPTOR*)(&req->Data[0]);
			std::cout << "Serial: " << sudis::base::ws_s(std::wstring(desc->bString)) << std::endl;
			device.m_serial = sudis::base::ws_s(std::wstring(desc->bString));


			/*Product*/
			ZeroMemory(buffer, BUFSIZE);
			req->ConnectionIndex = j;
			req->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8) + coninfo.DeviceDescriptor.iProduct;
			req->SetupPacket.wLength = BUFSIZE - sizeof(USB_DESCRIPTOR_REQUEST);
			req->SetupPacket.wIndex = 0x409; //US English
			if(!DeviceIoControl(_handle, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
				&buffer, sizeof(buffer), &buffer, sizeof(buffer), &bytes_read, 0))
			{
				std::cerr << "Failed iProduct request by DeviceIoControl() from port: " << j << " errorCpde: " << GetLastError() << std::endl;
				//continue;
				device.m_isBlocked = true;
			}
			desc = (USB_STRING_DESCRIPTOR*)(&req->Data[0]);
			std::cout << "Product: " << sudis::base::ws_s(std::wstring(desc->bString)) << std::endl;
			device.m_product = sudis::base::ws_s(std::wstring(desc->bString));

			/*Vendor*/
			ZeroMemory(buffer, BUFSIZE);
			req->ConnectionIndex = j;
			req->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8) + coninfo.DeviceDescriptor.iManufacturer;
			req->SetupPacket.wLength = BUFSIZE - sizeof(USB_DESCRIPTOR_REQUEST);
			req->SetupPacket.wIndex = 0x409; //US English
			if (!DeviceIoControl(_handle, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
				&buffer, sizeof(buffer), &buffer, sizeof(buffer), &bytes_read, 0))
			{
				std::cerr << "Failed iManufacturer request by DeviceIoControl() from port: " << j << " errorCode: " << GetLastError() << std::endl;
				//continue;
				device.m_isBlocked = true;
			}
			desc = (USB_STRING_DESCRIPTOR*)(&req->Data[0]);
			std::cout << "Vendor: " << sudis::base::ws_s(std::wstring(desc->bString)) << std::endl;
			device.m_vendor = sudis::base::ws_s(std::wstring(desc->bString));

			device.m_diData = _diData;

			_devList.emplace_back(device);
		}
	}

	StorageControl::StorageControl()
	{
		m_classGuid = &GUID_DEVINTERFACE_USB_HUB;

		m_deviceInfoHandle = SetupDiGetClassDevsA(m_classGuid, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (m_deviceInfoHandle == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Failed SetupDiGetClassDevsA(). errorCode: " << GetLastError() << std::endl;
			throw std::runtime_error("Failed SetupDiGetClassDevsA().");
		}
	}

	StorageControl::~StorageControl()
	{
		SetupDiDestroyDeviceInfoList(m_deviceInfoHandle);
	}

	std::pair<bool, std::vector<Device>> StorageControl::getDevices(bool _isBlocked)
	{
		std::vector<Device> result;

		for (int deviceIndex = 0; ; deviceIndex++)
		{
			std::cout << std::endl;

			SP_DEVICE_INTERFACE_DATA deviceInterface = { 0 };
			deviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

			// Перечисление
			if (!SetupDiEnumDeviceInterfaces(m_deviceInfoHandle, 0, m_classGuid, deviceIndex, &deviceInterface))
			{
				std::cout << "End of enum by SetupDiEnumDeviceInterfaces(). errorCode: " << GetLastError() << std::endl;
				break;
			}

			// Вычисление буфера
			DWORD cbRequired = 0;
			SetupDiGetDeviceInterfaceDetailA(
				m_deviceInfoHandle,
				&deviceInterface,
				0,
				0,
				&cbRequired,
				0);
			if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
			{
				std::cerr << "No ERROR_INSUFFICIENT_BUFFER errorCode:" << GetLastError() << std::endl;
				continue;
			}

			std::vector<char> buffer(cbRequired, 0);
			PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetail =
				(PSP_DEVICE_INTERFACE_DETAIL_DATA)buffer.data();
			deviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			SP_DEVINFO_DATA diData;
			diData.cbSize = sizeof(SP_DEVINFO_DATA);

			// Повторный вызов для возврата данных в буфер
			if (!SetupDiGetDeviceInterfaceDetailA(
				m_deviceInfoHandle,
				&deviceInterface,
				deviceInterfaceDetail,
				cbRequired,
				&cbRequired,
				&diData))
			{
				std::cerr << "Failed obtain SP_DEVICE_INTERFACE_DETAIL_DATA by SetupDiGetDeviceInterfaceDetailA(). errorCode: " << GetLastError() << std::endl;
				continue;
			}

			/*Открываем устройство для отправки IOCTL*/
			HANDLE handle = CreateFile(deviceInterfaceDetail->DevicePath, GENERIC_WRITE, FILE_SHARE_WRITE,
				0, OPEN_EXISTING, 0, 0);

			if (handle == INVALID_HANDLE_VALUE)
			{
				std::cerr << "Failed CreateFile(). errorCode: " << GetLastError() << " continue." << std::endl;
				continue;
			}

			// получаем число портов на концентраторе
			USB_NODE_INFORMATION	nodeinfo {};
			nodeinfo.NodeType = UsbHub;
			DWORD bytes_read = 0;
			if (!DeviceIoControl(handle, IOCTL_USB_GET_NODE_INFORMATION,
				&nodeinfo, sizeof(nodeinfo), 
				&nodeinfo, sizeof(nodeinfo), 
				&bytes_read, 0))
			{
				std::cerr << "Failed IOCTL_USB_GET_NODE_INFORMATION request by DeviceIoControl() errorCode: " << GetLastError() << std::endl;
				continue;
			}

			UINT ports = (UINT)nodeinfo.u.HubInformation.HubDescriptor.bNumberOfPorts;
			std::cout << "Numbers of ports: " << ports << std::endl;
			getDeviceFromPort(ports, handle, diData, result);
		}

		return { true, result };
	}

	bool getDiData(const HDEVINFO hDevInfo, const Device& _device, SP_DEVINFO_DATA& _out)
	{
		for (DWORD index = 0; ; index++)
		{
			SP_DEVINFO_DATA devInfoData{0};
			devInfoData.cbSize = sizeof(devInfoData);
			if (!SetupDiEnumDeviceInfo(hDevInfo, index, &devInfoData))
			{
				if (GetLastError() == ERROR_NO_MORE_ITEMS)
					std::cout << "End of enum" << std::endl;
				else
					std::cerr << "SetupDiEnumDeviceInfo() failed. code: " << GetLastError() << std::endl;
				break;
			}
			std::vector<char> sb(1, '\0');
			DWORD requiredSize = 0;
			// 1ый вызов чтобы расширить буфер
			auto res = SetupDiGetDeviceInstanceIdA(hDevInfo, &devInfoData, sb.data(), sb.size(), &requiredSize);
			if (res == FALSE)
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					sb.resize(requiredSize);
					// 2ой вызов - получение данных
					res = SetupDiGetDeviceInstanceIdA(hDevInfo, &devInfoData, sb.data(), sb.size(), &requiredSize);
				}
				else
				{
					std::cerr << "Something goes netak. errorCode: " << GetLastError() << std::endl;
					return false;
				}
			}

			if (res == FALSE)
			{
				std::cerr << "Something goes netak. errorCode: " << GetLastError() << std::endl;
				return false;
			}

			std::string sSb(sb.data());

			std::string PID = toUpper(_device.m_pid);
			std::string VID = toUpper(_device.m_vid);
			std::string SERIAL = toUpper(_device.m_serial);
			sSb = toUpper(sSb);
			if (sSb.find("USB") != std::string::npos)
			{
				std::cout << "instanceId: " << sSb << std::endl;
				
				if (sSb.find(PID) != std::string::npos &&
					sSb.find(VID) != std::string::npos &&
					sSb.find(SERIAL) != std::string::npos)
				{
					_out = devInfoData;
					return true;
				}
			}
		}

		return false;
	}

	void StorageControl::enableDevice(const Device& _device, const bool _enable)
	{
		// Пробуем получить хендл XXX_USB_DEVICE а не XXX_USB_HUB
		const GUID* pDevInterfaceGuid = &GUID_DEVINTERFACE_USB_DEVICE;
		//const GUID* pDevInterfaceGuid = &_device.m_diData.ClassGuid;
		HDEVINFO hDevInfo = SetupDiGetClassDevsA(pDevInterfaceGuid, NULL, NULL, DIGCF_PRESENT 
			//| DIGCF_DEVICEINTERFACE 
			| DIGCF_ALLCLASSES);
		if (hDevInfo == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Failed SetupDiGetClassDevsA(). errorCode: " << GetLastError() << std::endl;
			return;
		}

		SP_DEVINFO_DATA diData;
		if (!getDiData(hDevInfo, _device, diData))
		{
			std::cerr << "Not found" << std::endl;
			return;
		}

		SP_PROPCHANGE_PARAMS params { 0 };
		params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		params.Scope = DICS_FLAG_GLOBAL;
		params.HwProfile = 0;
		if (_enable)
			params.StateChange = DICS_ENABLE;
		else
			params.StateChange = DICS_DISABLE;

		//SP_DEVINFO_DATA diData = _device.m_diData;
		//auto result = SetupDiSetClassInstallParamsA(m_deviceInfoHandle, &diData, (PSP_CLASSINSTALL_HEADER)&params, sizeof(params));
		auto result = SetupDiSetClassInstallParamsA(hDevInfo, &diData, (PSP_CLASSINSTALL_HEADER)&params, sizeof(params));
		if (result == false)
		{
			std::cerr << "SetupDiSetClassInstallParamsA() failed. code: " << GetLastError() << std::endl;
			return;
		}

		//result = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, m_deviceInfoHandle, &diData);
		result = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &diData);
		if (result == false)
		{
			std::cerr << "SetupDiCallClassInstaller() failed. code: " << GetLastError() << std::endl;
			return;
		}
	}

	bool StorageControl::blockDevice(const Device& _device)
	{
		enableDevice(_device, false);
		return true;
	}

	bool StorageControl::unblockDevice(const Device& _device)
	{
		enableDevice(_device, true);
		return true;
	}

	DWORD NotifyCallBack(
		HCMNOTIFICATION       hNotify,
		PVOID             Context,
		CM_NOTIFY_ACTION      Action,
		PCM_NOTIFY_EVENT_DATA EventData,
		DWORD                 EventDataSize)
	{
		std::cout << __FUNCTION__ << std::endl;

		return 0;
	}

	void StorageControl::registerPlugEvent()
	{
		//CM_NOTIFY_FILTER filter { 0 };
		//filter.cbSize = sizeof(CM_NOTIFY_FILTER);
		//filter.Flags = CM_NOTIFY_FILTER_FLAG_ALL_DEVICE_INSTANCES;
		//filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
		////filter.u.DeviceInterface.ClassGuid = GUID_DEVINTERFACE_USB_DEVICE;

		CM_NOTIFY_FILTER NotifyFilter = { 0 };
		NotifyFilter.cbSize = sizeof(NotifyFilter);
		NotifyFilter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
		NotifyFilter.u.DeviceInterface.ClassGuid = GUID_DEVINTERFACE_USB_DEVICE;

		PCM_NOTIFY_CALLBACK callback = NotifyCallBack;
		HCMNOTIFICATION notifyContext;
		auto res = CM_Register_Notification(
			//&filter,
			&NotifyFilter,
			(PVOID)"Context",
			callback,
			&notifyContext);

		if (res == CR_SUCCESS)
			std::cout << "Successing callback registered" << std::endl;
		else
		{
			std::cerr << "Failed register callback. errorCode: " << GetLastError() << " res: " << res << std::endl;
		}
	}
}

#endif