cmake_minimum_required(VERSION 3.15...3.30)

project(
	operating_systems_laboratories_4
	VERSION 1.0
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 26)

add_library(common STATIC
	sources/common/PidFile.cpp
)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sources)

add_executable(leaker sources/leaker/main.cpp)
target_link_libraries(leaker common)

add_executable(mapper sources/mapper/main.cpp)

add_executable(monitor sources/monitor/main.cpp)
target_link_libraries(monitor common)

install(
	DIRECTORY
	DESTINATION /usr/share/operating_systems_laboratories_4/dumps
	DIRECTORY_PERMISSIONS
	OWNER_WRITE OWNER_READ
)
install(
	TARGETS
	leaker mapper monitor
	DESTINATION /usr/share/operating_systems_laboratories_4/bin
)
install(
	FILES install/operating_systems_laboratories_4.service install/operating_systems_laboratories_4.timer
	DESTINATION /etc/systemd/system
)