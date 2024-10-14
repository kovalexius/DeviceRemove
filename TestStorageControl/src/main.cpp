#include <iostream>


#include <StorageControl.h>
#include "test.h"


void callback(sudis::storage_control::Device& _dev)
{
	std::cout << std::endl << __FUNCTION__ << 
		" New device. pid: " << _dev.m_pid <<
		" vid: " << _dev.m_vid <<
		" serial: " << _dev.m_serial <<
		" product: " << _dev.m_product <<
		" vendor: " << _dev.m_vendor <<
		(_dev.m_isBlocked ? " is blocked " : " is unblocked ") <<
		std::endl;


	//sudis::storage_control::StorageControl devControl;
	//auto [res, devList] = devControl.getDevices();
	//if (res)
	//{
	//	for (const auto& device : devList)
	//	{
	//		std::cout << "Device is " << (device.m_isBlocked ? "blocked" : "unblocked") <<
	//			" PID: " << device.m_pid <<
	//			" VID: " << device.m_vid <<
	//			" serial: " << device.m_serial << std::endl;
	//		//devControl.blockDevice(device);
	//	}
	//}
}

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "Rus");
	
	//std::this_thread::sleep_for(std::chrono::seconds(10));

	sudis::storage_control::StorageControl devControl;
	//auto [res, devList] = devControl.getDevices();

	//if (res)
	//{
	//	for (const auto& device : devList)
	//	{
	//		std::cout << "Device is " << (device.m_isBlocked ? "blocked" : "unblocked") <<
	//			" PID: " << device.m_pid <<
	//			" VID: " << device.m_vid <<
	//			" serial: " << device.m_serial << std::endl;
	//		//devControl.blockDevice(device);
	//		//sudis::storage_control::testGetProperties(device);
	//	}
	//}
	
	devControl.registerPlugEvent(callback);

	//test();

	std::getchar();

	return 0;
}