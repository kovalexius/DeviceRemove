#ifdef __linux
#include "InputDevices.h"
#include <libudev.h>
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <cstring>
#include <mutex>

InputDevices::InputDevices() : m_run(true)
{}
InputDevices::~InputDevices() {
    std::cout<<"DESTRUCTOR!"<<std::endl;
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_run.store(false);
    }
    if (m_thread.joinable())
        m_thread.join();
}

void InputDevices::startCheck()
{
  m_thread = std::thread([this]() {
        checkDevices();
      });
}
bool InputDevices::isRunning() const {
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_run.load();
}

std::vector<std::unique_ptr<Device>> const& InputDevices::getDevices() const {
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_devices;
}

void InputDevices::viewDevices() {
    std::lock_guard<std::mutex> lock(m_mtx);
    int i = 0;
    for (const auto& device : m_devices) {
        ++i;
        std::cout << i << std::endl;
        std::cout << "Device Path: " << device->strdevpath << std::endl;
        std::cout << "Product: " << device->product << std::endl;
    }
}

void InputDevices::checkDevices() {
    auto udev = udev_new();
    if (!udev) {
        std::cerr << "Can't create udev\n";
        {
            std::lock_guard<std::mutex> lock(m_mtx);
           m_run.store(false);
        }
        return;
    }

    std::vector<std::unique_ptr<Device>> tempVecDev;

    do{
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (!m_run.load())
                break;
        }

        auto enumerate = udev_enumerate_new(udev);
        if (!enumerate) {
            std::cerr << "Can't create udev enumerator\n";
            break;
        }

        udev_enumerate_add_match_subsystem(enumerate, "usb");
        udev_enumerate_scan_devices(enumerate);
        struct udev_list_entry* dev_item;

        tempVecDev.clear();
        udev_list_entry_foreach(dev_item, udev_enumerate_get_list_entry(enumerate)) {
            udev_device* dev = udev_device_new_from_syspath(udev, udev_list_entry_get_name(dev_item));
            if (!dev)
                continue;

            std::unique_ptr<Device> devic1 = std::make_unique<Device>();
            const char* devnode = udev_device_get_devnode(dev);
            const char* product = udev_device_get_property_value(dev, "ID_MODEL");

            const char* devtype = udev_device_get_devtype(dev);
            if (devtype && product) {
                devic1->strdevpath = devnode ? devnode : "";
                devic1->product = product ? product : "";
                tempVecDev.emplace_back(std::move(devic1));
            }
            udev_device_unref(dev);
        }
        udev_enumerate_unref(enumerate);

            m_devices.clear();
            m_devices.insert(m_devices.end(),std::make_move_iterator(tempVecDev.begin()), std::make_move_iterator(tempVecDev.end()));
            viewDevices();
          std::this_thread::sleep_for(std::chrono::seconds(3));
    } while(true);

    udev_unref(udev);
}
#endif
