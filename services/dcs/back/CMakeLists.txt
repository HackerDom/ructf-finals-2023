cmake_minimum_required(VERSION 3.16)
project(back)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")

set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
add_subdirectory(googletest)
add_subdirectory(glog)

set(CPPHTTPLIB_USE_POLL ON)
add_subdirectory(cpp-httplib)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(lang)

