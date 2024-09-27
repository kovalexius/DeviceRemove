#include <iostream>

#include <StorageControl.h>
#include "test.h"

int main()
{
	sudis::storage_control::StorageControl devControl;
	auto [res, devList] = devControl.getDevices(false);

	//if (res)
	//{
	//	for(const auto& device : devList)
	//		devControl.blockDevice(device);
	//}

	devControl.registerPlugEvent();

	std::getchar();

	//test();

	return 0;
}