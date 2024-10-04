#include <iostream>


#include <StorageControl.h>
#include "test.h"


int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "Rus");

	sudis::storage_control::StorageControl devControl;
	auto [res, devList] = devControl.getDevices(false);

	if (res)
	{
		for (const auto& device : devList)
		{
			std::cout << "Device is " << (device.m_isBlocked ? "blocked" : "unblocked") <<
				" PID: " << device.m_pid <<
				" VID: " << device.m_vid <<
				" serial: " << device.m_serial << std::endl;
			//devControl.blockDevice(device);
		}
	}

	//devControl.registerPlugEvent();

	//test();

	std::getchar();

	return 0;
}