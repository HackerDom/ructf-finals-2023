cmake_minimum_required(VERSION 3.16)
project(client)

add_executable(dcsclient src/main.cpp)
target_include_directories(dcsclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dcsclient common)
