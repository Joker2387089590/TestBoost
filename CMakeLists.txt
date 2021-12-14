cmake_minimum_required(VERSION 3.18)

project(TestBoost)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Utility)

include(FindBoost)
find_package(Boost
	COMPONENTS
		system
	REQUIRED)

find_package(Threads REQUIRED)

add_library(TestBoost STATIC
	Udp.h
	Udp.cpp
	# main.cpp
	# TcpClients.h
	# TcpClients.cpp
)

target_link_libraries(TestBoost
	PUBLIC
		Boost::boost
		Boost::system
		Threads::Threads)

add_executable(Read read.cpp)

target_link_libraries(Read
	PUBLIC
		TestBoost)

add_executable(Write write.cpp)

target_link_libraries(Write
	PUBLIC
		TestBoost)
