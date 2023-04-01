cmake_minimum_required(VERSION 3.16)
project(common)

set(SOURCES
        src/utils/files/files.cpp
        src/utils/process/process.cpp
        src/utils/strings/strings.cpp
        )

set(HEADERS
        include/utils/defer/defer.h
        include/utils/files/files.h
        include/utils/process/process.h
        include/utils/strings/strings.h
        )

add_library(common ${SOURCES} ${HEADERS})

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
