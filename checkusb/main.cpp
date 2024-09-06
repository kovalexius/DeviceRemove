#include <iostream>
#include <memory>
#include "InputDevices.h"
using namespace std;


int main() {
    setlocale(LC_ALL, "RUS");
    std::unique_ptr<InputDevices> obj = std::make_unique<InputDevices>();
   //obj->checkDevices2();
   //obj->viewDevices();
    obj->startCheck();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    const char* mystr = "STOOOOP!";
    std::cout<<mystr<<std::endl;
    std::cin.get();
    const auto& devs = obj->getDevices();
  //  for (const auto& dev : devs) {
  //      std::cout << "Device Path: " << dev->m_strdevpath << std::endl;
  //      std::cout << "Product: " << dev->m_product << std::endl;
  //      std::cout << "Class: " << dev->m_classDev<< std::endl;
		//std::cout << "DevType: " << std::hex << dev->m_devType << std::dec << std::endl;
  //  }
    return 0;
}
