project(StorageControl)

add_library(StorageControl STATIC
	./src/StorageControl.h
	./src/StorageControlWindows.cpp
	./src/StrWindows.cpp
	./src/Str.h
	./src/RaiiWrapper.cpp
	./src/RaiiWrapper.h
	./src/Thread.h
	./src/Event.h
)

if(WIN32)
target_link_libraries(StorageControl PUBLIC
	Setupapi
	Cfgmgr32
)
endif()

target_include_directories(StorageControl PUBLIC
	./src/
)