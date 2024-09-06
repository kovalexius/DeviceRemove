#ifdef _WIN32
#include <iostream>
#include "StorageControl.h"


#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <usbiodef.h>

namespace sudis::storage_control
{
	std::vector<Device> StorageControl::getDevices(bool _isBlocked)
	{
		//const GUID* classGuid = &GUID_DEVINTERFACE_USB_DEVICE;
		const GUID* classGuid = &GUID_DEVINTERFACE_DISK;

		DWORD flags = DIGCF_PRESENT;
		if (_isBlocked)
			flags |= DIGCF_ALLCLASSES;
		else
			flags |= DIGCF_DEVICEINTERFACE;

		HDEVINFO deviceInfoHandle = SetupDiGetClassDevsA(classGuid, NULL, NULL, flags);
		if (deviceInfoHandle == INVALID_HANDLE_VALUE)
			return {};

		for (int devIndex = 0; ; devIndex++)
		{
			SP_DEVICE_INTERFACE_DATA deviceInterface = { 0 };
			deviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			auto res = SetupDiEnumDeviceInterfaces(deviceInfoHandle, 0, &GUID_DEVINTERFACE_DISK, devIndex, &deviceInterface);
			if (res == false)
				return {};

			DWORD cbRequired = 0;
			SetupDiGetDeviceInterfaceDetailA(deviceInfoHandle, &deviceInterface, 0, 0, &cbRequired, 0);
			if (ERROR_INSUFFICIENT_BUFFER != GetLastError())	// Всегда должна вернуть какую нибудь ошибку
				continue;

			std::vector<char> detailBuffer(cbRequired, 0);
			PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(detailBuffer.data());
			deviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			
			res = SetupDiGetDeviceInterfaceDetailA(deviceInfoHandle, &deviceInterface, deviceInterfaceDetail, cbRequired, &cbRequired, 0);
			if (res == false)
				continue;

			std::cout << "DevicePath: " << deviceInterfaceDetail->DevicePath << std::endl;
		}

		
	}
}

#endif