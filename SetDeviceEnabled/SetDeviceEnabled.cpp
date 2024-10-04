//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// взято отсюда https://ru.stackoverflow.com/questions/891397/%D0%92%D0%BA%D0%BB%D1%8E%D1%87%D0%B5%D0%BD%D0%B8%D0%B5-%D0%BE%D1%82%D0%BA%D0%BB%D1%8E%D1%87%D0%B5%D0%BD%D0%B8%D0%B5-usb-%D0%BF%D0%BE%D1%80%D1%82%D0%BE%D0%B2-%D0%B8%D0%BB%D0%B8-%D1%83%D1%81%D1%82%D1%80%D0%BE%D0%B9%D1%81%D1%82%D0%B2
/// и отсюда https://stackoverflow.com/questions/1438371/win32-api-function-to-programmatically-enable-disable-device
/// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <Usbiodef.h>


enum DiFunction
{
	PropertyChange = (int)0x12,
	EnableClass = (int)0x13
};

enum StateChangeAction
{
	Enable = 1,
	Disable = 2,
	PropChange = 3,
	Start = 4,
	Stop = 5
};

enum Scopes
{
	Global = 1,
	ConfigSpecific = 2,
	ConfigGeneral = 4
};

struct PropertyChangeParameters
{
	int Size;
	// part of header. It's flattened out into 1 structure.
	DiFunction DiFunction;
	StateChangeAction StateChange;
	Scopes Scope;
	int HwProfile;
};

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

std::vector<SP_DEVINFO_DATA> GetDeviceInfoData(const HDEVINFO _handle)
{
	std::vector<SP_DEVINFO_DATA> result;
	for (DWORD index = 0; ; index++)
	{
		SP_DEVINFO_DATA devInfoData;
		devInfoData.cbSize = sizeof(devInfoData);
		if (!SetupDiEnumDeviceInfo(_handle, index, &devInfoData))
		{
			auto code = GetLastError();
			if (code != ERROR_NO_MORE_ITEMS)
				std::cerr << "SetupDiEnumDeviceInfo() failed. code: " << code << std::endl;
			break;
		}
		result.push_back(devInfoData);
	}
	return result;
}

int GetIndexOfInstance(const HDEVINFO _handle, /*const */ std::vector<SP_DEVINFO_DATA>& _dataList, const std::string& _instanceId)
{
	for (int i = 0; i < _dataList.size(); i++)
	{
		std::vector<char> sb(1, '\0');
		DWORD requiredSize = 0;
		auto res = SetupDiGetDeviceInstanceIdA(_handle, &_dataList[i], sb.data(), sb.size(), &requiredSize);
		if (res == FALSE)
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				sb.resize(requiredSize);
				res = SetupDiGetDeviceInstanceIdA(_handle, &_dataList[i], sb.data(), sb.size(), &requiredSize);
			}
			else
				return -1;
		}

		if (res == FALSE)
			return -1;

		std::cout << "Class GUID: " << fromGuid(_dataList[i].ClassGuid) <<
			" DevInst: " << _dataList[i].DevInst << std::endl;

		std::string sSb(sb.data());
		std::cout << "instanceId: " << sSb << std::endl;
		if (_instanceId.compare(sSb) == 0)
			return i;
	}

	return -1;
}

void EnableDevice(const HDEVINFO _handle, /*const*/ SP_DEVINFO_DATA& _diData, bool _enable)
{
	PropertyChangeParameters params;
	params.Size = 8;
	params.DiFunction = DiFunction::PropertyChange;
	params.Scope = Scopes::Global;
	if (_enable)
		params.StateChange = StateChangeAction::Enable;
	else
		params.StateChange = StateChangeAction::Disable;

	//SP_CLASSINSTALL_HEADER params;
	auto result = SetupDiSetClassInstallParamsA(_handle, &_diData, (PSP_CLASSINSTALL_HEADER)&params, sizeof(params));
	if (result == false)
	{
		std::cout << "SetupDiSetClassInstallParamsA() failed. code: " << GetLastError() << std::endl;
		return;
	}

	result = SetupDiCallClassInstaller(DiFunction::PropertyChange, _handle, &_diData);
	if (result == false)
	{
		std::cout << "SetupDiCallClassInstaller() failed. code: " << GetLastError() << std::endl;
		return;
	}
}

///
/// @param _classGuid The class guid of the device. Available in the device manager.
/// @param _instanceId The device instance id of the device. Available in the device manager.
/// @param _enable True to enable, False to disable.
void SetDeviceEnabled(const GUID* _classGuid, std::string& _instanceId, bool _enable)
{
	HDEVINFO diSetHandle = SetupDiGetClassDevsA(_classGuid, NULL, NULL, DIGCF_PRESENT
		// | DIGCF_DEVICEINTERFACE 
		 | DIGCF_ALLCLASSES
	);
	std::vector<SP_DEVINFO_DATA> diData = GetDeviceInfoData(diSetHandle);

	int index = GetIndexOfInstance(diSetHandle, diData, _instanceId);
	if (index < 0)
	{
		std::cout << "Instance not found." << std::endl;
		return;
	}
	EnableDevice(diSetHandle, diData[index], _enable);
}



int main()
{
	//std::string instanceId("USB\\Unknown123");
	std::string instanceId("USB\\VID_8564&PID_1000\\CCYYMMDDHHMMSSXTHBWP");
	//SetDeviceEnabled(&GUID_DEVINTERFACE_USB_DEVICE, instanceId, false);
	
	//std::cout << "USB:" << std::endl;
	//SetDeviceEnabled(&GUID_DEVINTERFACE_USB_DEVICE, instanceId, true);
	std::cout << std::endl << "DISK:" << std::endl;
	SetDeviceEnabled(&GUID_DEVINTERFACE_DISK, instanceId, false);
	

	return 0;
}