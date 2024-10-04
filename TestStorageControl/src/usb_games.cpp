///
/// https://kaimi.io/2012/07/windows-usb-monitoring/
/// 

#include <set>
#include <stdexcept>

#include <Windows.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>

#include "usb_games.h"


//Структура device_info описана выше
const usb_monitor::device_info usb_monitor::get_device_info(char letter)
{
	//Формируем строку вида \\.\X: для устройства
	wchar_t volume_access_path[] = L"\\\\.\\X:";
	volume_access_path[4] = static_cast<wchar_t>(letter);

	//Открываем его
	HANDLE vol = CreateFileW(volume_access_path, 0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	//Если ошибка - бросим исключение
	if (vol == INVALID_HANDLE_VALUE)
		throw usb_monitor_exception("Cannot open device");

	//Теперь надо получить номер устройства
	STORAGE_DEVICE_NUMBER sdn;
	DWORD bytes_ret = 0;
	long DeviceNumber = -1;

	//Это делается таким IOCTL-запросом к устройству
	if (DeviceIoControl(vol,
		IOCTL_STORAGE_GET_DEVICE_NUMBER,
		NULL, 0, &sdn, sizeof(sdn),
		&bytes_ret, NULL))
		DeviceNumber = sdn.DeviceNumber;

	//Хендл нам больше не нужен
	CloseHandle(vol);

	//Если номер не получен - ошибка
	if (DeviceNumber == -1)
		throw usb_monitor_exception("Cannot get device number");

	// Еще две вспомогательные строки вида X: 
	wchar_t devname[] = L"?:";
	wchar_t devpath[] = L"?:\\";
	devname[0] = static_cast<wchar_t>(letter);
	devpath[0] = static_cast<wchar_t>(letter);
	wchar_t dos_name[MAX_PATH + 1];
	//Этот момент уже описан выше - используется для определения
	//флешек и флопиков
	if (!QueryDosDeviceW(devname, dos_name, MAX_PATH))
		throw usb_monitor_exception("Cannot get device info");

	bool floppy = std::wstring(dos_name).find(L"\\Floppy") != std::wstring::npos;
	//Определяем тип устройства
	UINT drive_type = GetDriveTypeW(devpath);

	const GUID* guid;

	//Теперь выясним класс устройства, с которым имеем дело
	switch (drive_type)
	{
	case DRIVE_REMOVABLE:
		if (floppy)
			guid = &GUID_DEVINTERFACE_FLOPPY; //флоппи
		else
			guid = &GUID_DEVINTERFACE_DISK; //какой-то диск
		break;

	case DRIVE_FIXED:
		guid = &GUID_DEVINTERFACE_DISK; //какой-то диск
		break;

	case DRIVE_CDROM:
		guid = &GUID_DEVINTERFACE_CDROM; //CD-ROM
		break;

	default:
		throw usb_monitor_exception("Unknown device"); //Неизвестный тип
	}

	//Получаем хендл к набору различных сведений о классе устройств info.dev_class на локальном компьютере,
	//выше эта функция уже была упомянута
	HDEVINFO dev_info = SetupDiGetClassDevsW(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	//Если что-то не так, кинем исключение
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

	//Готовимся найти наш девайс через перечисление
	while (true)
	{
		//Перечисляем все устройства заданного класса
		ret = SetupDiEnumDeviceInterfaces(dev_info, NULL, guid, index, &spdid);
		if (!ret)
			break;

		//Получаем размер данных об устройстве
		size = 0;
		SetupDiGetDeviceInterfaceDetailW(dev_info, &spdid, NULL, 0, &size, NULL);

		if (size != 0 && size <= sizeof(buf))
		{
			pspdidd->cbSize = sizeof(*pspdidd);

			ZeroMemory(reinterpret_cast<PVOID>(&spdd), sizeof(spdd));
			spdd.cbSize = sizeof(spdd);

			//А теперь получаем информацию об устройстве
			BOOL res = SetupDiGetDeviceInterfaceDetailW(dev_info, &spdid, pspdidd, size, &size, &spdd);
			if (res)
			{
				//Если все окей, открываем девайс по пути, который узнали
				HANDLE drive = CreateFileW(pspdidd->DevicePath, 0,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, 0, NULL);
				if (drive != INVALID_HANDLE_VALUE)
				{
					//Получаем номер устройства, и если он совпадает
					//с определенным нами ранее,
					//то нужное устройство мы нашли
					STORAGE_DEVICE_NUMBER sdn;
					DWORD bytes_returned = 0;
					if (DeviceIoControl(drive,
						IOCTL_STORAGE_GET_DEVICE_NUMBER,
						NULL, 0, &sdn, sizeof(sdn),
						&bytes_returned, NULL))
					{
						if (DeviceNumber == static_cast<long>(sdn.DeviceNumber))
						{
							//Если нашли, то выходим из цикла
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

	//А если не нашли устройство - то кинем эксепшен
	if (!found)
		throw usb_monitor_exception("Cannot find device");

	//Находим родителя устройства
	//Например, USB-хаб для флешки
	DEVINST dev_parent = 0;
	if (CR_SUCCESS != CM_Get_Parent(&dev_parent, spdd.DevInst, 0))
		throw usb_monitor_exception("Cannot get device parent");

	//Заполняем нашу структуру всякой интересной
	//информацией об устройстве
	device_info info;
	info.dev_class = *guid;
	info.dev_inst = dev_parent;
	info.dev_number = DeviceNumber;

	//И возвращаем ее
	return info;
}

std::set<wchar_t> usb_monitor::get_flash_disks(bool include_usb_hard_drives)
{
	std::set<wchar_t> devices;

	//Получаем список логических разделов
	unsigned int disks = GetLogicalDrives();

	//Строка для формирования имен вида A:, B:, ...
	wchar_t drive_root[] = L"?:";

	//Смотрим, какие логические разделы есть в системе
	for (int i = 31; i >= 0; i--)
	{
		//Если диск есть
		if (disks & (1 << i))
		{
			//Формируем строку с именем диска
			drive_root[0] = static_cast<wchar_t>('A') + i;
			//Получаем тип устройства
			DWORD type = GetDriveTypeW(drive_root);

			//Если это съемный девайс (флешка или флоппи)
			if (type == DRIVE_REMOVABLE)
			{
				//Получаем тип девайса - это, похоже, самый простой
				//путь отличить флешку от флоппика
				wchar_t buf[MAX_PATH];
				if (QueryDosDeviceW(drive_root, buf, MAX_PATH))
					if (std::wstring(buf).find(L"\\Floppy") == std::wstring::npos) //Если в имени нет подстроки "\\Floppy",
						devices.insert(static_cast<wchar_t>('A') + i); //то это флешка
			}
			//Если это какой-то жесткий диск, и мы их тоже мониторим
			else if (type == DRIVE_FIXED && include_usb_hard_drives)
			{
				try
				{
					//Получаем информацию о девайсе
					device_info info(get_device_info('A' + i));
					//Получаем хендл к набору различных сведений о классе устройств info.dev_class на локальном компьютере
					//Подробнее читайте MSDN, а функция get_device_info описана ниже
					HDEVINFO dev_info = SetupDiGetClassDevsW(&info.dev_class, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

					//Если хендл получен
					if (dev_info != INVALID_HANDLE_VALUE)
					{
						SP_DEVINFO_DATA dev_data;
						dev_data.cbSize = sizeof(dev_data);
						//Получаем информацию о жестком диске
						if (SetupDiEnumDeviceInfo(dev_info, info.dev_number, &dev_data))
						{
							DWORD properties;
							//Получаем информацию о свойстве SPDRP_REMOVAL_POLICY жесткого диска
							//Оно говорит о том, может ли устройство быть извлечено
							//Если может, добавим его в результирующий набор
							if (SetupDiGetDeviceRegistryPropertyW(dev_info, &dev_data, SPDRP_REMOVAL_POLICY, NULL, reinterpret_cast<PBYTE>(&properties), sizeof(properties), NULL)
								&&
								properties != CM_REMOVAL_POLICY_EXPECT_NO_REMOVAL)
								devices.insert(static_cast<wchar_t>('A') + i);
						}

						//Освободим информационный хендл
						SetupDiDestroyDeviceInfoList(dev_info);
					}
				}
				catch (const usb_monitor_exception&)
				{
					//Ошибки тут игнорируем
				}
			}
		}
	}

	//Возвращаем набор
	return devices;
}
