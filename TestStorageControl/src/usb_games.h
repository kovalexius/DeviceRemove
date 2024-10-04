#pragma once

class usb_monitor
{
	//Вспомогательная структура для получения всякой информации об устройстве
	struct device_info
	{
		DEVINST dev_inst;
		GUID dev_class;
		long dev_number;
	};

	class usb_monitor_exception : public std::runtime_error
	{
	public:
		explicit usb_monitor_exception(const std::string& message) 
			: std::runtime_error(message)
		{}
	};

public:

	//Структура device_info описана выше
	const device_info usb_monitor::get_device_info(char letter);

	std::set<wchar_t> get_flash_disks(bool include_usb_hard_drives);
};