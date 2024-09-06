###
### И сюда посмотреть http://forum.oszone.net/nextoldesttothread-93961.html
### и сюда https://otvet.mail.ru/question/42833829
###

cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 17)

project(DeviceRemove)

###
### WmDeviceMsgMy
###
add_executable(WmDeviceMsgMy
	WmDeviceMsgMy/WmDeviceMsgMy.cpp
)
#set_target_properties(WmDeviceMsgMy PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
#set_target_properties(WmDeviceMsgMy PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")

###
### DetectingMediaInsertionOrRemoval
###
add_executable(DetectingMediaInsertionOrRemoval
	DetectingMediaInsertionOrRemoval/DetectingMediaInsertionOrRemoval.cpp
)

###
### ProcessingARequestToRemoveADevice
###
add_executable(ProcessingARequestToRemoveADevice
	ProcessingARequestToRemoveADevice/ProcessingARequestToRemoveADevice.cpp
)


###
### RemoveDriveByLetter
###
add_executable(RemoveDriveByLetter
	RemoveDriveByLetter/RemoveDriveByLetter.cpp
)
target_link_libraries(RemoveDriveByLetter
	Setupapi.lib
)

###
### USBitc
###
#add_executable(USBitc
#	USBitc/Project1.cpp
#	USBitc/Unit2.cpp
#	USBitc/Unit2.h
#)

###
### checkusb by Дмитрий Болоцкий
###
add_executable(checkusb
	checkusb/InputDevices.cpp
	checkusb/InputDevices.h
	checkusb/LinInputDevices.cpp
	checkusb/main.cpp
)
target_link_libraries(checkusb  PRIVATE Setupapi)

###
### SetDeviceEnabled
###
add_executable(SetDeviceEnabled
	SetDeviceEnabled/SetDeviceEnabled.cpp
)

target_link_libraries(SetDeviceEnabled PRIVATE Setupapi)


###
### StorageControl lib
###
add_subdirectory(StorageControl)
add_subdirectory(TestStorageControl)
