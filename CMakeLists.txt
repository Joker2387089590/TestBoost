cmake_minimum_required(VERSION 3.18)

project(TestBoost)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(BOOST_ROOT $ENV{BOOST_ROOT})
# set(Boost_USE_STATIC_LIBS ON)
# include(FindBoost)
# find_package(Boost
# 	COMPONENTS
# 		system
# 		date_time
# 	REQUIRED)
# message("Boost Libraries: ${Boost_LIBRARIES}")

# add_executable(TestBoost main.cpp)
# target_include_directories(TestBoost
# 	PRIVATE
# 		${Boost_INCLUDE_DIR})
# target_link_libraries(TestBoost
# 	PRIVATE
# 		Boost::system
# 		Boost::date_time)

add_executable(TestBoost main.cpp)
