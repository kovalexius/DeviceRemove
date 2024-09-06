project(StorageControl)

add_library(StorageControl STATIC
	./src/StorageControl.h
	./src/StorageControlWindows.cpp
)

if(WIN32)
target_link_libraries(StorageControl PUBLIC
	Setupapi
)
endif()

target_include_directories(StorageControl PUBLIC
	./src/
)