#ifdef _WIN32
#include "InputDevices.h"
#include <string>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>
#include <Windows.h>
#include <cfgmgr32.h> 
#include <SetupAPI.h>
#include <initguid.h>
#include <Usbiodef.h>
#include <Usbioctl.h>
#include <Devpkey.h> 

InputDevices::InputDevices():
m_run(false)
// : m_logCtx(0, "InputDevices") {}
{}

InputDevices::~InputDevices() {
    std::cout << "Destructor!" << std::endl;
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_run.store(false);
    }

    if (m_thread.joinable())
        m_thread.join();
    std::cout << "Destructor! Stop m_thread" << std::endl;
}

bool InputDevices::isRunning() const
{
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_run.load();
}

const std::vector<Device>& InputDevices::getDevices() const
{
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_devices;
}

void deviceInfo ( HDEVINFO& hDevInfo, SP_DEVINFO_DATA& _deviceInfoData, Device& _dev)
{
    char buffer[MAX_PATH];

	// Получаем описание устройства
	if (SetupDiGetDeviceRegistryProperty(
		hDevInfo,
		&_deviceInfoData,
		SPDRP_DEVICEDESC,
		nullptr,
		reinterpret_cast<PBYTE>(buffer),
		sizeof(buffer),
		nullptr))
	{
		_dev.m_product = buffer;
	}

    if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &_deviceInfoData, SPDRP_CLASS, nullptr, (PBYTE)buffer, sizeof(buffer), nullptr))
    {
        //std::cout << "Class: " << buffer << std::endl;
       _dev.m_classDev = buffer;
    }
	DWORD devType;
	if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &_deviceInfoData, SPDRP_DEVTYPE, nullptr, (PBYTE)&devType, sizeof(devType), nullptr))
	{
		_dev.m_devType = devType;
	}
	else
	{
		std::cout << "SetupDiGetDeviceRegistryPropertyA(SPDRP_DEVTYPE) failed. code: " << GetLastError() << std::endl;
	}
}

void InputDevices::startCheck() {
    std::cout << "Starting checkDevices thread" << std::endl;
    m_thread = std::thread([this]() {
        checkDevices();
        });
}

bool IsMouseDevice(DEVINST devInst)
{
    ULONG status;
    CONFIGRET cr = CM_Get_DevNode_Status(&status, nullptr, devInst, 0);
    if (cr == CR_SUCCESS)
    {
        if (status & DN_DRIVER_LOADED)
        {
            // Device has a loaded driver
            char szClass[MAX_PATH];
            ULONG ulSize = sizeof(szClass);
            cr = CM_Get_DevNode_Registry_Property(devInst, CM_DRP_CLASS, nullptr, szClass, &ulSize, 0);
            if (cr == CR_SUCCESS && strcmp(szClass, "Mouse") == 0)
            {
                return true;
            }
        }
    }
    return false;
}

void InputDevices::checkDevices()
{
	SP_DEVINFO_DATA DeviceInfoData;
	char szDeviceInstanceID[MAX_DEVICE_ID_LEN];

    m_run.store(true);
	do
	{
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (!m_run.load())
                break;
        }
		// Получаем список USB устройств
		//auto hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, nullptr, nullptr, DIGCF_PRESENT
		HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, nullptr, nullptr, DIGCF_PRESENT 
			| DIGCF_DEVICEINTERFACE
			//| DIGCF_ALLCLASSES
		);
        if (hDevInfo == INVALID_HANDLE_VALUE)
        {
            m_run.store(false);
            break;
        }
		// Перебираем устройства
		m_devices.clear();
		for (int i = 0;; i++)
		{
            Device dev;
			DeviceInfoData.cbSize = sizeof(DeviceInfoData);
			if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData))
				break;

			auto status = CM_Get_Device_ID(DeviceInfoData.DevInst, szDeviceInstanceID, MAX_PATH, 0);
			if (status != CR_SUCCESS)
				continue;
			dev.m_strdevpath = szDeviceInstanceID;
			deviceInfo(hDevInfo, DeviceInfoData, dev); 
			m_devices.emplace_back(std::move(dev));
		}

		// Выводим информацию о найденных устройствах
		viewDevices();
		std::this_thread::sleep_for(std::chrono::seconds(20));
	} while (true);
}


void InputDevices::viewDevices()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    int i{ 0 };
    for (const auto& item : m_devices)
    {
        i++;
        std::cout << i << "\n";
        std::cout << "Device Path: " << item.m_product << std::endl;
        std::cout << "Product: " << item.m_strdevpath << std::endl;
        std::cout << "Class: " << item.m_classDev << std::endl;
		std::cout << "DevType: " << std::hex << item.m_devType << std::dec << std::endl;
    }
	std::cout << std::endl;
}

#endif // _WIN32