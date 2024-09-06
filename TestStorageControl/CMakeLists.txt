project(TestStorageControl)

add_executable(TestStorageControl
	src/main.cpp
	src/test.cpp
	src/test.h
)

target_link_libraries(TestStorageControl
	PRIVATE
		StorageControl
)