cmake_minimum_required(VERSION 3.16)
project(back)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(googletest)
set(BUILD_TESTING OFF)
add_subdirectory(glog)

add_executable(dcs
        src/main.cpp
        src/compiler/lexer.cpp
        src/compiler/parser.cpp
        src/compiler/ast.cpp
        src/utils/utils.h
        src/utils/utils.cpp
        src/compiler/compiler.cpp
        src/compiler/translator.cpp
        src/storage/storage.cpp
        )
target_link_libraries(dcs glog::glog)
target_include_directories(dcs PUBLIC src)

add_executable(dcs_tests
        src/tests/lexer_tests.cpp
        src/tests/parser_tests.cpp
        src/compiler/lexer.cpp
        src/compiler/parser.cpp
        src/compiler/ast.cpp
        src/compiler/compiler.cpp
        src/utils/utils.cpp
        src/tests/compiler_tests.cpp
        src/tests/functional_tests.cpp
        src/compiler/translator.cpp
        src/tests/storage_tests.cpp
        src/storage/storage.cpp)
target_include_directories(dcs_tests PUBLIC src)
target_link_libraries(dcs_tests GTest::gtest_main glog::glog)
include(GoogleTest)
gtest_discover_tests(dcs_tests)
