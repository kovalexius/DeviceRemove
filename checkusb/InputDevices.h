#pragma once

#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

#include <Windows.h>


struct Device
{
    std::string m_strdevpath;
    std::string m_product;
    std::string m_classDev;
    std::string m_manuFacture;
    std::string m_server;
	DWORD m_devType;
};

class InputDevices
{
    std::atomic<bool> m_run;
    std::thread m_thread;
    mutable std::mutex m_mtx;

    void checkDevices();
public:
    std::vector<Device> m_devices;
    InputDevices();
    ~InputDevices();

	const std::vector<Device>& getDevices() const;
     void startCheck();
    void viewDevices();
    bool isRunning() const;
};
