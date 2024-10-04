///
/// https://kaimi.io/2012/07/windows-usb-monitoring/
/// 

#include <set>
#include <stdexcept>

#include <Windows.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>

#include "usb_games.h"


//��������� device_info ������� ����
const usb_monitor::device_info usb_monitor::get_device_info(char letter)
{
	//��������� ������ ���� \\.\X: ��� ����������
	wchar_t volume_access_path[] = L"\\\\.\\X:";
	volume_access_path[4] = static_cast<wchar_t>(letter);

	//��������� ���
	HANDLE vol = CreateFileW(volume_access_path, 0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	//���� ������ - ������ ����������
	if (vol == INVALID_HANDLE_VALUE)
		throw usb_monitor_exception("Cannot open device");

	//������ ���� �������� ����� ����������
	STORAGE_DEVICE_NUMBER sdn;
	DWORD bytes_ret = 0;
	long DeviceNumber = -1;

	//��� �������� ����� IOCTL-�������� � ����������
	if (DeviceIoControl(vol,
		IOCTL_STORAGE_GET_DEVICE_NUMBER,
		NULL, 0, &sdn, sizeof(sdn),
		&bytes_ret, NULL))
		DeviceNumber = sdn.DeviceNumber;

	//����� ��� ������ �� �����
	CloseHandle(vol);

	//���� ����� �� ������� - ������
	if (DeviceNumber == -1)
		throw usb_monitor_exception("Cannot get device number");

	// ��� ��� ��������������� ������ ���� X: 
	wchar_t devname[] = L"?:";
	wchar_t devpath[] = L"?:\\";
	devname[0] = static_cast<wchar_t>(letter);
	devpath[0] = static_cast<wchar_t>(letter);
	wchar_t dos_name[MAX_PATH + 1];
	//���� ������ ��� ������ ���� - ������������ ��� �����������
	//������ � ��������
	if (!QueryDosDeviceW(devname, dos_name, MAX_PATH))
		throw usb_monitor_exception("Cannot get device info");

	bool floppy = std::wstring(dos_name).find(L"\\Floppy") != std::wstring::npos;
	//���������� ��� ����������
	UINT drive_type = GetDriveTypeW(devpath);

	const GUID* guid;

	//������ ������� ����� ����������, � ������� ����� ����
	switch (drive_type)
	{
	case DRIVE_REMOVABLE:
		if (floppy)
			guid = &GUID_DEVINTERFACE_FLOPPY; //������
		else
			guid = &GUID_DEVINTERFACE_DISK; //�����-�� ����
		break;

	case DRIVE_FIXED:
		guid = &GUID_DEVINTERFACE_DISK; //�����-�� ����
		break;

	case DRIVE_CDROM:
		guid = &GUID_DEVINTERFACE_CDROM; //CD-ROM
		break;

	default:
		throw usb_monitor_exception("Unknown device"); //����������� ���
	}

	//�������� ����� � ������ ��������� �������� � ������ ��������� info.dev_class �� ��������� ����������,
	//���� ��� ������� ��� ���� ���������
	HDEVINFO dev_info = SetupDiGetClassDevsW(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	//���� ���-�� �� ���, ����� ����������
	if (dev_info == INVALID_HANDLE_VALUE)
		throw usb_monitor_exception("Cannot get device class");

	DWORD index = 0;
	BOOL ret = FALSE;

	BYTE buf[1024];
	PSP_DEVICE_INTERFACE_DETAIL_DATA_W pspdidd = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(buf);
	SP_DEVICE_INTERFACE_DATA spdid;
	SP_DEVINFO_DATA spdd;
	DWORD size;

	spdid.cbSize = sizeof(spdid);

	bool found = false;

	//��������� ����� ��� ������ ����� ������������
	while (true)
	{
		//����������� ��� ���������� ��������� ������
		ret = SetupDiEnumDeviceInterfaces(dev_info, NULL, guid, index, &spdid);
		if (!ret)
			break;

		//�������� ������ ������ �� ����������
		size = 0;
		SetupDiGetDeviceInterfaceDetailW(dev_info, &spdid, NULL, 0, &size, NULL);

		if (size != 0 && size <= sizeof(buf))
		{
			pspdidd->cbSize = sizeof(*pspdidd);

			ZeroMemory(reinterpret_cast<PVOID>(&spdd), sizeof(spdd));
			spdd.cbSize = sizeof(spdd);

			//� ������ �������� ���������� �� ����������
			BOOL res = SetupDiGetDeviceInterfaceDetailW(dev_info, &spdid, pspdidd, size, &size, &spdd);
			if (res)
			{
				//���� ��� ����, ��������� ������ �� ����, ������� ������
				HANDLE drive = CreateFileW(pspdidd->DevicePath, 0,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, 0, NULL);
				if (drive != INVALID_HANDLE_VALUE)
				{
					//�������� ����� ����������, � ���� �� ���������
					//� ������������ ���� �����,
					//�� ������ ���������� �� �����
					STORAGE_DEVICE_NUMBER sdn;
					DWORD bytes_returned = 0;
					if (DeviceIoControl(drive,
						IOCTL_STORAGE_GET_DEVICE_NUMBER,
						NULL, 0, &sdn, sizeof(sdn),
						&bytes_returned, NULL))
					{
						if (DeviceNumber == static_cast<long>(sdn.DeviceNumber))
						{
							//���� �����, �� ������� �� �����
							CloseHandle(drive);
							found = true;
							break;
						}
					}

					CloseHandle(drive);
				}
			}
		}
		index++;
	}

	SetupDiDestroyDeviceInfoList(dev_info);

	//� ���� �� ����� ���������� - �� ����� ��������
	if (!found)
		throw usb_monitor_exception("Cannot find device");

	//������� �������� ����������
	//��������, USB-��� ��� ������
	DEVINST dev_parent = 0;
	if (CR_SUCCESS != CM_Get_Parent(&dev_parent, spdd.DevInst, 0))
		throw usb_monitor_exception("Cannot get device parent");

	//��������� ���� ��������� ������ ����������
	//����������� �� ����������
	device_info info;
	info.dev_class = *guid;
	info.dev_inst = dev_parent;
	info.dev_number = DeviceNumber;

	//� ���������� ��
	return info;
}

std::set<wchar_t> usb_monitor::get_flash_disks(bool include_usb_hard_drives)
{
	std::set<wchar_t> devices;

	//�������� ������ ���������� ��������
	unsigned int disks = GetLogicalDrives();

	//������ ��� ������������ ���� ���� A:, B:, ...
	wchar_t drive_root[] = L"?:";

	//�������, ����� ���������� ������� ���� � �������
	for (int i = 31; i >= 0; i--)
	{
		//���� ���� ����
		if (disks & (1 << i))
		{
			//��������� ������ � ������ �����
			drive_root[0] = static_cast<wchar_t>('A') + i;
			//�������� ��� ����������
			DWORD type = GetDriveTypeW(drive_root);

			//���� ��� ������� ������ (������ ��� ������)
			if (type == DRIVE_REMOVABLE)
			{
				//�������� ��� ������� - ���, ������, ����� �������
				//���� �������� ������ �� ��������
				wchar_t buf[MAX_PATH];
				if (QueryDosDeviceW(drive_root, buf, MAX_PATH))
					if (std::wstring(buf).find(L"\\Floppy") == std::wstring::npos) //���� � ����� ��� ��������� "\\Floppy",
						devices.insert(static_cast<wchar_t>('A') + i); //�� ��� ������
			}
			//���� ��� �����-�� ������� ����, � �� �� ���� ���������
			else if (type == DRIVE_FIXED && include_usb_hard_drives)
			{
				try
				{
					//�������� ���������� � �������
					device_info info(get_device_info('A' + i));
					//�������� ����� � ������ ��������� �������� � ������ ��������� info.dev_class �� ��������� ����������
					//��������� ������� MSDN, � ������� get_device_info ������� ����
					HDEVINFO dev_info = SetupDiGetClassDevsW(&info.dev_class, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

					//���� ����� �������
					if (dev_info != INVALID_HANDLE_VALUE)
					{
						SP_DEVINFO_DATA dev_data;
						dev_data.cbSize = sizeof(dev_data);
						//�������� ���������� � ������� �����
						if (SetupDiEnumDeviceInfo(dev_info, info.dev_number, &dev_data))
						{
							DWORD properties;
							//�������� ���������� � �������� SPDRP_REMOVAL_POLICY �������� �����
							//��� ������� � ���, ����� �� ���������� ���� ���������
							//���� �����, ������� ��� � �������������� �����
							if (SetupDiGetDeviceRegistryPropertyW(dev_info, &dev_data, SPDRP_REMOVAL_POLICY, NULL, reinterpret_cast<PBYTE>(&properties), sizeof(properties), NULL)
								&&
								properties != CM_REMOVAL_POLICY_EXPECT_NO_REMOVAL)
								devices.insert(static_cast<wchar_t>('A') + i);
						}

						//��������� �������������� �����
						SetupDiDestroyDeviceInfoList(dev_info);
					}
				}
				catch (const usb_monitor_exception&)
				{
					//������ ��� ����������
				}
			}
		}
	}

	//���������� �����
	return devices;
}
