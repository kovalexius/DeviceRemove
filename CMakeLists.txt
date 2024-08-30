cmake_minimum_required(VERSION 3.26)

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


### И сюда посмотреть http://forum.oszone.net/nextoldesttothread-93961.html