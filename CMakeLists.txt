cmake_minimum_required(VERSION 3.26)

project(DeviceRemove)

add_executable(WmDeviceMsg
	WmDeviceMsg/main.cpp
)
#set_target_properties(WmDeviceMsg PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
#set_target_properties(WmDeviceMsg PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")

add_executable(RemoveDriveByLetter
	RemoveDriveByLetter/main.cpp
)

target_link_libraries(RemoveDriveByLetter
	Setupapi.lib
)