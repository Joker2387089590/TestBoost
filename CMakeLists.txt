cmake_minimum_required(VERSION 3.21)
project(Net)

### Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Pre-install
# set(CMAKE_SKIP_BUILD_RPATH FALSE)
# set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

### Package
include(FindBoost)
find_package(Boost
	COMPONENTS
		system
	REQUIRED)

find_package(Threads REQUIRED)

### Sources
add_subdirectory(Utility)

add_library(NetDependency STATIC Common.h Common.cpp)
target_link_libraries(NetDependency
	PUBLIC
		Boost::boost
		Boost::system
		Threads::Threads
		Utility)

add_library(Udp STATIC Udp.h Udp.cpp)
target_link_libraries(Udp PUBLIC NetDependency)

add_library(Tcp STATIC TcpClients.h TcpClients.cpp)
target_link_libraries(Tcp PUBLIC NetDependency)

add_executable(Read read.cpp)
target_link_libraries(Read PUBLIC Udp)

add_executable(Write write.cpp)
target_link_libraries(Write PUBLIC Udp)

### Install
install(TARGETS Read Write RUNTIME_DEPENDENCIES)
