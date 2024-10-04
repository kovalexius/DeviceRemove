project(TestStorageControl)

add_executable(TestStorageControl
	src/main.cpp
	src/test.cpp
	src/test.h
	src/usb_games.cpp
	src/usb_games.h
)

target_link_libraries(TestStorageControl
	PRIVATE
		StorageControl
)