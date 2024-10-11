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

	std::string fromGuid(const GUID& _guid)
	{
		std::stringstream ss;
		ss << std::setfill('0') << std::hex << std::uppercase <<
			std::setfill('0') << std::setw(8) << _guid.Data1 << "-" <<
			std::setfill('0') << std::setw(4) << _guid.Data2 << "-" <<
			std::setfill('0') << std::setw(4) << _guid.Data3 << "-" <<
			std::setfill('0') << std::setw(2) << (int)_guid.Data4[0] <<
			std::setfill('0') << std::setw(2) << (int)_guid.Data4[1] << "-" <<
			std::setfill('0') << std::setw(2) << (int)_guid.Data4[2] <<
			std::setfill('0') << std::setw(2) << (int)_guid.Data4[3] <<
			std::setfill('0') << std::setw(2) << (int)_guid.Data4[4] <<
			std::setfill('0') << std::setw(2) << (int)_guid.Data4[5] <<
			std::setfill('0') << std::setw(2) << (int)_guid.Data4[6] <<
			std::setfill('0') << std::setw(2) << (int)_guid.Data4[7];

		return ss.str();
	}

	/// 
	/// @brief Получить список конечных устройств с USB разветлителя
	/// @param _handle файл устройства разветвителя
	void getDeviceFromPort(const HANDLE _handle, std::vector<Device>& _devList)
	{
		// получаем число портов на концентраторе
		USB_NODE_INFORMATION	nodeinfo{};
		nodeinfo.NodeType = UsbHub;
		DWORD bytes_read = 0;
		if (!DeviceIoControl(_handle, IOCTL_USB_GET_NODE_INFORMATION,
			&nodeinfo, sizeof(nodeinfo),
			&nodeinfo, sizeof(nodeinfo),
			&bytes_read, 0))
		{
			std::cerr << "Failed IOCTL_USB_GET_NODE_INFORMATION request by DeviceIoControl() errorCode: " << GetLastError() << std::endl;
			return;
		}

		UINT ports = (UINT)nodeinfo.u.HubInformation.HubDescriptor.bNumberOfPorts;
		std::cout << "Numbers of ports: " << ports << std::endl;

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
				continue; //нет устройства
			}

			Device device;

			std::cout << std::endl << "bDescriptorType: " << (int)coninfo.DeviceDescriptor.bDescriptorType <<
				" bcdUSB :" << coninfo.DeviceDescriptor.bcdUSB <<
				" bDeviceClass: " << (int)coninfo.DeviceDescriptor.bDeviceClass <<
				" bDeviceSubClass: " << (int)coninfo.DeviceDescriptor.bDeviceSubClass <<
				" bDeviceProtocol: " << (int)coninfo.DeviceDescriptor.bDeviceProtocol <<
				" bMaxPacketSize0: " << (int)coninfo.DeviceDescriptor.bMaxPacketSize0 <<
				" bcdDevice: " << coninfo.DeviceDescriptor.bcdDevice << std::endl;

			std::cout << "VID: " << std::hex << (int)coninfo.DeviceDescriptor.idVendor << " PID: " << (int)coninfo.DeviceDescriptor.idProduct << std::dec << std::endl;

			device.m_vid = int_to_hex((int)coninfo.DeviceDescriptor.idVendor, 4);
			device.m_pid = int_to_hex((int)coninfo.DeviceDescriptor.idProduct, 4);

			const UINT BUFSIZE = 1000;
			char buffer[BUFSIZE] = { 0 };
			USB_DESCRIPTOR_REQUEST* req = (USB_DESCRIPTOR_REQUEST*)&buffer;


			///* Тест USB_DEVICE_DESCRIPTOR_TYPE */
			ZeroMemory(buffer, BUFSIZE);
			req->ConnectionIndex = j;
			req->SetupPacket.wValue = (USB_DEVICE_DESCRIPTOR_TYPE << 8);
			req->SetupPacket.wLength = BUFSIZE - sizeof(USB_DESCRIPTOR_REQUEST);
			req->SetupPacket.wIndex = 0x409;			//US English
			if (!DeviceIoControl(_handle, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
				&buffer, sizeof(buffer), &buffer, sizeof(buffer), &bytes_read, 0))
			{
				std::cerr << "Failed USB_DEVICE_DESCRIPTOR_TYPE request by DeviceIoControl() from port: " << j << " errorCode: " << GetLastError() << std::endl;
				continue;
			}
			USB_DEVICE_DESCRIPTOR* devDescr = (USB_DEVICE_DESCRIPTOR*)&req->Data[0];
			std::cout << "bDescriptorType: " << (int)devDescr->bDescriptorType <<
				" bcdUSB :" << devDescr->bcdUSB <<
				" bDeviceClass: " << (int)devDescr->bDeviceClass <<
				" bDeviceSubClass: " << (int)devDescr->bDeviceSubClass <<
				" bDeviceProtocol: " << (int)devDescr->bDeviceProtocol <<
				" bMaxPacketSize0: " << (int)devDescr->bMaxPacketSize0 <<
				" idVendor: "		<< (int)devDescr->idVendor <<
				" idProduct: "		<< (int)devDescr->idProduct <<
				" bcdDevice: "		<< (int)devDescr->bcdDevice << 
				" iManufacturer: "	<< (int)devDescr->iManufacturer <<
				" iProduct: "		<< (int)devDescr->iProduct <<
				" iSerialNumber: "	<< (int)devDescr->iSerialNumber <<
				" bNumConfigurations: " << (int)devDescr->bNumConfigurations <<
				std::endl;

			/*Serial number*/
			ZeroMemory(buffer, BUFSIZE);
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

			_devList.emplace_back(device);
		}
	}

	StorageControl::StorageControl()
	{
	}

	StorageControl::~StorageControl()
	{
	}

	std::pair<bool, std::vector<Device>> StorageControl::getDevices()
	{
		std::vector<Device> result;

		{
			const GUID* classGuid = &GUID_DEVINTERFACE_USB_HUB;
			sudis::base::HDevInfo deviceInfoHandle = SetupDiGetClassDevsA(classGuid, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
			if (deviceInfoHandle.getRef() == INVALID_HANDLE_VALUE)
			{
				std::cerr << "Failed SetupDiGetClassDevsA(). errorCode: " << GetLastError() << std::endl;
				throw std::runtime_error("Failed SetupDiGetClassDevsA().");
			}

			for (int deviceIndex = 0; ; deviceIndex++)
			{
				std::cout << std::endl;

				SP_DEVICE_INTERFACE_DATA deviceInterface = { 0 };
				deviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

				// Перечисление
				if (!SetupDiEnumDeviceInterfaces(deviceInfoHandle.getRef(), 0, classGuid, deviceIndex, &deviceInterface))
				{
					std::cout << "End of enum by SetupDiEnumDeviceInterfaces(). errorCode: " << GetLastError() << std::endl;
					break;
				}

				// Вычисление буфера
				DWORD cbRequired = 0;
				SetupDiGetDeviceInterfaceDetailA(
					deviceInfoHandle.getRef(),
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
					deviceInfoHandle.getRef(),
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

				getDeviceFromPort(handle, result);
			}
		}

		/// Фильтрация по носителям информации
		{
			sudis::base::HDevInfo hDevInfo = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_DISK,
				NULL,
				NULL,
				DIGCF_PRESENT
				| DIGCF_ALLCLASSES);

			for (auto it = result.begin(); it != result.end(); )
			{
				SP_DEVINFO_DATA diData;
				if (!getDiData(hDevInfo, *it, diData))
					it = result.erase(it);
				else
					++it;
			}
		}

		/// Заблочен или нет, вернуть свойства
		//{
		//	sudis::base::HDevInfo hDevInfo1 = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT
		//		| DIGCF_ALLCLASSES);
		//	for (auto item : result)
		//	{
		//		SP_DEVINFO_DATA diData;
		//		if (getDiData(hDevInfo1, item, diData))
		//		{
		//			SP_PROPCHANGE_PARAMS params{ 0 };
		//			params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		//			params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

		//			DWORD ClassInstallParamsSize = sizeof(SP_PROPCHANGE_PARAMS);
		//			DWORD requiredSize = 0;
		//			auto result = SetupDiGetClassInstallParamsA(hDevInfo1.getRef(), &diData, (PSP_CLASSINSTALL_HEADER)&params, ClassInstallParamsSize, &requiredSize);
		//			if (result == FALSE)
		//			{
		//				std::cerr << "Fail to get SP_PROPCHANGE_PARAMS. errorCode: " << std::hex << GetLastError() << std::endl;
		//			}
		//		}
		//	}
		//}

		return { true, result };
	}

	bool getDiData(const sudis::base::HDevInfo& hDevInfo, const Device& _device, SP_DEVINFO_DATA& _out)
	{
		for (DWORD index = 0; ; index++)
		{
			SP_DEVINFO_DATA devInfoData{0};
			devInfoData.cbSize = sizeof(devInfoData);
			if (!SetupDiEnumDeviceInfo(hDevInfo.getRef(), index, &devInfoData))
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
			auto res = SetupDiGetDeviceInstanceIdA(hDevInfo.getRef(), &devInfoData, sb.data(), sb.size(), &requiredSize);
			if (res == FALSE)
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					sb.resize(requiredSize);
					// 2ой вызов - получение данных
					res = SetupDiGetDeviceInstanceIdA(hDevInfo.getRef(), &devInfoData, sb.data(), sb.size(), &requiredSize);
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

	void testGetProperties(const Device& _device)
	{
		//sudis::base::HDevInfo hDevInfo = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_DISK, NULL, NULL, DIGCF_PRESENT
		//	| DIGCF_ALLCLASSES);
		sudis::base::HDevInfo hDevInfo = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT
			| DIGCF_ALLCLASSES);
		if (hDevInfo.getRef() == INVALID_HANDLE_VALUE)
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
		
		SP_CLASSINSTALL_HEADER header{ 0 };
		header.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		DWORD requiredSize = 0;
		//if (!SetupDiSetClassInstallParamsW(hDevInfo.getRef(), &diData, &header, sizeof(SP_CLASSINSTALL_HEADER)))
		//{
		//	std::cerr << "Fail to set SP_CLASSINSTALL_HEADER. errorCode: " << std::hex << GetLastError() << std::endl;
 	//	}
		//else
		//{
		//	std::cout << "SP_PROPCHANGE_PARAMS success" << std::endl;
		//}

		SP_ADDPROPERTYPAGE_DATA AddPropertyPageData{0};
		ZeroMemory(&AddPropertyPageData, sizeof(SP_ADDPROPERTYPAGE_DATA));
		AddPropertyPageData.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		AddPropertyPageData.ClassInstallHeader.InstallFunction = DIF_ADDPROPERTYPAGE_ADVANCED;
		DWORD ClassInstallParamsSize = sizeof(SP_ADDPROPERTYPAGE_DATA);
		if (!SetupDiGetClassInstallParamsW(hDevInfo.getRef(), &diData, (PSP_CLASSINSTALL_HEADER)&AddPropertyPageData, ClassInstallParamsSize, &requiredSize))
		{
			std::cerr << "Fail to get SP_ADDPROPERTYPAGE_DATA. errorCode: " << std::hex << GetLastError() << std::endl;
		}
		else
		{
			std::cout << "SP_PROPCHANGE_PARAMS success" << std::endl;
		}

		SP_PROPCHANGE_PARAMS params1{ 0 };
		params1.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		params1.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		ClassInstallParamsSize = sizeof(SP_PROPCHANGE_PARAMS);
		requiredSize = 0;
		if (!SetupDiGetClassInstallParamsW(hDevInfo.getRef(), &diData, (PSP_CLASSINSTALL_HEADER)&params1, ClassInstallParamsSize, &requiredSize))
		{
			std::cerr << "Fail to get SP_PROPCHANGE_PARAMS. errorCode: " << std::hex << GetLastError() << std::endl;
		}
		else
		{
			std::cout << "SP_PROPCHANGE_PARAMS success" << std::endl;
		}
	}

	void enableDevice(const Device& _device, const bool _enable)
	{
		{
			sudis::base::HDevInfo hDevInfo = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT
				//| DIGCF_DEVICEINTERFACE
				| DIGCF_ALLCLASSES);
			if (hDevInfo.getRef() == INVALID_HANDLE_VALUE)
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

			SP_PROPCHANGE_PARAMS params{ 0 };
			params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
			params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
			params.Scope = DICS_FLAG_GLOBAL;
			params.HwProfile = 0;
			if (_enable)
				params.StateChange = DICS_ENABLE;
			else
				params.StateChange = DICS_DISABLE;

			auto result = SetupDiSetClassInstallParamsA(hDevInfo.getRef(), &diData, (PSP_CLASSINSTALL_HEADER)&params, sizeof(params));
			if (result == false)
			{
				std::cerr << "SetupDiSetClassInstallParamsA() failed. code: " << GetLastError() << std::endl;
				return;
			}

			result = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo.getRef(), &diData);
			if (result == false)
			{
				std::cerr << "SetupDiCallClassInstaller() failed. code: " << GetLastError() << std::endl;
				return;
			}
		}

		//{
			//sudis::base::HDevInfo hDevInfo = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT
			//	| DIGCF_ALLCLASSES);
			//if (hDevInfo.getRef() == INVALID_HANDLE_VALUE)
			//{
			//	std::cerr << "Failed SetupDiGetClassDevsA(). errorCode: " << GetLastError() << std::endl;
			//	return;
			//}

			//SP_DEVINFO_DATA diData;
			//if (!getDiData(hDevInfo, _device, diData))
			//{
			//	std::cerr << "Not found" << std::endl;
			//	return;
			//}

			//SP_PROPCHANGE_PARAMS params1{ 0 };
			//params1.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
			//params1.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
			//params1.Scope = DICS_FLAG_GLOBAL;
			//params1.HwProfile = 0;

			//DWORD ClassInstallParamsSize = sizeof(SP_PROPCHANGE_PARAMS);
			//DWORD requiredSize = 0;
			//auto result1 = SetupDiGetClassInstallParamsA(hDevInfo.getRef(), &diData, (PSP_CLASSINSTALL_HEADER)&params1, ClassInstallParamsSize, &requiredSize);
			//if (result1 == FALSE)
			//{
			//	std::cerr << "Fail to get SP_PROPCHANGE_PARAMS. errorCode: " << std::hex << GetLastError() << std::endl;
			//}
			//else
			//{
			//	std::cout << "SP_PROPCHANGE_PARAMS success" << std::endl;
			//}
		//}
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

	DWORD NotifyBlockedDevices(
		HCMNOTIFICATION			_hNotify,
		PVOID					_context,
		CM_NOTIFY_ACTION		_action,
		PCM_NOTIFY_EVENT_DATA	_eventData,
		DWORD					_eventDataSize)
	{
		/// Фильтрация по USB устройствам
		GUID expectedGuid = { 0x00530055L, 0x0042, 0x005C, { 0x56, 0x00, 0x49, 0x00, 0x44, 0x00, 0x5F, 0x00 } };
		if (_eventData->u.DeviceInterface.ClassGuid == expectedGuid)
		{
			std::cout << std::endl << __FUNCTION__ << std::endl;
			//std::cout << "Context: " << (char*)Context << std::endl;
			std::cout << "Action: " << _action << std::endl;
			std::cout << "EventDataSize: " << _eventDataSize << std::endl;
			std::cout << "EventGuid: " << fromGuid(_eventData->u.DeviceHandle.EventGuid) << std::endl;
			std::cout << "DataSize: " << _eventData->u.DeviceHandle.DataSize << std::endl;
			//std::cout << "Data: " << std::string((char*)&EventData->u.DeviceHandle.Data[0], EventData->u.DeviceHandle.DataSize) << std::endl;
			std::cout << "NameOffset: " << _eventData->u.DeviceHandle.NameOffset << std::endl;
			std::cout << "ClassGuid: " << fromGuid(_eventData->u.DeviceInterface.ClassGuid) << std::endl;
			std::cout << "SymbolicLink: " << sudis::base::ws_s(_eventData->u.DeviceInterface.SymbolicLink) << std::endl;
			std::cout << "DeviceInstance: " << sudis::base::ws_s(_eventData->u.DeviceInstance.InstanceId) << std::endl;
			std::cout << "FilterType: " << _eventData->FilterType << std::endl;

			if (_context)
			{
				StorageControl* obj = (StorageControl*)_context;
				DevInst inst;
				inst.m_instanceId = sudis::base::ws_s(_eventData->u.DeviceInstance.InstanceId);
				if (_action == CM_NOTIFY_ACTION_DEVICEINSTANCEENUMERATED)
					inst.m_action = Actions::ACTION_DEVICEINSTANCEENUMERATED;
				else if (_action == CM_NOTIFY_ACTION_DEVICEINSTANCESTARTED)
					inst.m_action = Actions::ACTION_DEVICEINSTANCESTARTED;
				else
				{
					std::cerr << "Unknown action: " << _action << std::endl;
					return ERROR_SUCCESS;		/// Всегда возвращать Success
				}
				obj->addDevice2Queue(inst);		/// Добавление события в очередь
			}
			else
				std::cerr << "Context is invalid" << std::endl;

		}

		return ERROR_SUCCESS;
	}

	void StorageControl::registerDevicesEvent()
	{
		CM_NOTIFY_FILTER NotifyFilter = { 0 };
		NotifyFilter.cbSize = sizeof(NotifyFilter);
		NotifyFilter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINSTANCE;
		NotifyFilter.Flags = CM_NOTIFY_FILTER_FLAG_ALL_DEVICE_INSTANCES;

		PCM_NOTIFY_CALLBACK callback = NotifyBlockedDevices;
		HCMNOTIFICATION notifyContext;
		auto res = CM_Register_Notification(
			&NotifyFilter,
			(PVOID)this,
			callback,
			&notifyContext);

		if (res == CR_SUCCESS)
			std::cout << "Successing callback registered" << std::endl;
		else
		{
			std::cerr << "Failed register callback. errorCode: " << GetLastError() << " res: " << res << std::endl;
		}
	}

	void StorageControl::registerPlugEvent(std::function<void(Device& _device)> _callback)
	{
		m_newDeviceCallback = _callback;
		registerDevicesEvent();

		Thread::start();
	}

	void StorageControl::addDevice2Queue(const DevInst& _inst)
	{
		/// Если не задали колбек то и смысла что-то добавлять в очередь нет
		if (!m_newDeviceCallback)
			return;

		std::unique_lock lk(m_devQMutex);
		m_devQueue.push_back(_inst);
	}

	void StorageControl::init()
	{
	}

	void StorageControl::unInit()
	{
	}

	uint32_t StorageControl::getSleepTimeout() const
	{
		return 100;
	}


	bool StorageControl::run()
	{
		/// Если функтор не задали, то нет смысла дальше производить обработку
		if (!m_newDeviceCallback)
			return true;

		DevInst inst;

		/// Потокобезопасное извлечение из очереди
		{
			std::unique_lock lk(m_devQMutex);	// ждем короткий миг. предложение: избавиться от мьютекса, использовать lock free двустороннюю очередь 
			if (m_devQueue.empty())
				return true;
			inst = m_devQueue.front();
			m_devQueue.pop_front();
		}

		/// Сопоставление instanceId с PID, VID, Serial
		auto [res, devList] = getDevices();
		if (!res)
		{
			std::cerr << "Empty list of USB storages" << std::endl;
			return true;
		}
		for (auto& item : devList)
		{
			std::string PID = toUpper(item.m_pid);
			std::string VID = toUpper(item.m_vid);
			std::string SERIAL = toUpper(item.m_serial);

			std::string instanceId = toUpper(inst.m_instanceId);

			if (instanceId.find(PID) != std::string::npos &&
				instanceId.find(VID) != std::string::npos &&
				instanceId.find(SERIAL) != std::string::npos)
				
				m_newDeviceCallback(item);
		}

		return true;
	}
}

#endif