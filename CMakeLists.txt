cmake_minimum_required(VERSION 3.18)

project(TestBoost)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FindBoost)
find_package(Boost
	COMPONENTS
		ALL
	REQUIRED)

add_executable(TestBoost
	Udp.h
	main.cpp
	Udp.cpp)
target_link_libraries(TestBoost
	PRIVATE
		Boost::boost
		Boost::system)
